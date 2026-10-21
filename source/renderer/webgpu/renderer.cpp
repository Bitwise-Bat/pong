#include "renderer/webgpu/renderer.hpp"

#include <algorithm>
#include <array>
#include <cstddef>

#include "assert.hpp"
#include "log.hpp"

namespace pong
{
    namespace
    {
        constexpr size_t INITIAL_RECT_CAPACITY = 256;

        constexpr const char* SHADER = R"(
struct Globals { size: vec2f };
@group(0) @binding(0) var<uniform> globals: Globals;

struct RectData
{
    rect: vec4f,
    color: u32,
};
@group(0) @binding(1) var<storage, read> rects: array<RectData>;

struct VsOut
{
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) uv: vec2f,
};

@vertex
fn vs_main(@builtin(vertex_index) index: u32) -> VsOut
{
    let data = rects[index / 4u];
    // Corners: (0,0) (1,0) (0,1) (1,1)
    let uv = vec2f(f32(index & 1u), f32((index >> 1u) & 1u));
    let pixel = data.rect.xy + uv * data.rect.zw;
    let ndc = vec2f(pixel.x / globals.size.x * 2.0 - 1.0, 1.0 - pixel.y / globals.size.y * 2.0);

    var out: VsOut;
    out.position = vec4f(ndc, 0.0, 1.0);
    out.color = unpack4x8unorm(data.color);
    out.uv = uv;
    return out;
}

@fragment
fn fs_main(in: VsOut) -> @location(0) vec4f
{
    let border_width : f32 = 0.025;
    
    let is_border = in.uv.x < border_width || in.uv.x > (1.0 - border_width) ||
                    in.uv.y < border_width || in.uv.y > (1.0 - border_width);

    if(is_border)
    {
        return vec4f(0, 0, 0, 1);
    }

    return in.color;
}
)";
    }

    WebGpuRenderer::WebGpuRenderer(uint32_t width, uint32_t height) : m_width(width), m_height(height)
    {
        m_instance = wgpu::CreateInstance(nullptr);
        P_ASSERT(m_instance, "WebGPU: failed to create instance");

        wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvas{};
        canvas.selector = "#canvas";

        wgpu::SurfaceDescriptor surface_desc{};
        surface_desc.nextInChain = &canvas;
        m_surface = m_instance.CreateSurface(&surface_desc);

        m_instance.RequestAdapter(
            nullptr, wgpu::CallbackMode::AllowSpontaneous,
            [this](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message)
            {
                if (status != wgpu::RequestAdapterStatus::Success)
                {
                    fatal_impl(std::format("WebGPU: adapter request failed: {}", std::string_view(message)));
                }
                request_device(std::move(adapter));
            });
    }

    void WebGpuRenderer::request_device(wgpu::Adapter adapter)
    {
        wgpu::SurfaceCapabilities caps{};
        m_surface.GetCapabilities(adapter, &caps);
        m_format = caps.formats[0];

        wgpu::DeviceDescriptor device_desc{};
        device_desc.SetUncapturedErrorCallback(
            [](const wgpu::Device&, wgpu::ErrorType, wgpu::StringView message)
            {
                P_LOG_INFO("WebGPU error: {}", std::string_view(message));
            });

        adapter.RequestDevice(
            &device_desc, wgpu::CallbackMode::AllowSpontaneous,
            [this](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message)
            {
                if (status != wgpu::RequestDeviceStatus::Success)
                {
                    fatal_impl(std::format("WebGPU: device request failed: {}", std::string_view(message)));
                }
                m_device = std::move(device);
                init_resources();
            });
    }

    void WebGpuRenderer::init_resources()
    {
        m_queue = m_device.GetQueue();

        wgpu::SurfaceConfiguration config{};
        config.device = m_device;
        config.format = m_format;
        config.width = m_width;
        config.height = m_height;
        config.alphaMode = wgpu::CompositeAlphaMode::Opaque;
        config.presentMode = wgpu::PresentMode::Fifo;
        m_surface.Configure(&config);

        wgpu::ShaderSourceWGSL wgsl{};
        wgsl.code = SHADER;
        wgpu::ShaderModuleDescriptor module_desc{};
        module_desc.nextInChain = &wgsl;
        wgpu::ShaderModule module = m_device.CreateShaderModule(&module_desc);

        wgpu::ColorTargetState target{};
        target.format = m_format;

        wgpu::FragmentState fragment{};
        fragment.module = module;
        fragment.entryPoint = "fs_main";
        fragment.targetCount = 1;
        fragment.targets = &target;

        wgpu::RenderPipelineDescriptor pipeline_desc{};
        pipeline_desc.vertex.module = module;
        pipeline_desc.vertex.entryPoint = "vs_main";
        pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipeline_desc.fragment = &fragment;
        m_pipeline = m_device.CreateRenderPipeline(&pipeline_desc);

        wgpu::BufferDescriptor uniform_desc{};
        uniform_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        uniform_desc.size = 16;
        m_uniform_buffer = m_device.CreateBuffer(&uniform_desc);

        const std::array<float, 4> globals{static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 0.0f};
        m_queue.WriteBuffer(m_uniform_buffer, 0, globals.data(), sizeof(globals));

        ensure_capacity(INITIAL_RECT_CAPACITY);
        m_ready = true;
    }

    void WebGpuRenderer::ensure_capacity(size_t rect_count)
    {
        if (rect_count <= m_rect_capacity) return;

        size_t capacity = std::max(m_rect_capacity, INITIAL_RECT_CAPACITY);
        while (capacity < rect_count) capacity *= 2;

        wgpu::BufferDescriptor rect_desc{};
        rect_desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        rect_desc.size = capacity * sizeof(RectData);
        m_rect_buffer = m_device.CreateBuffer(&rect_desc);

        std::array<wgpu::BindGroupEntry, 2> entries{};
        entries[0].binding = 0;
        entries[0].buffer = m_uniform_buffer;
        entries[0].size = 16;
        entries[1].binding = 1;
        entries[1].buffer = m_rect_buffer;
        entries[1].size = rect_desc.size;

        wgpu::BindGroupDescriptor bind_group_desc{};
        bind_group_desc.layout = m_pipeline.GetBindGroupLayout(0);
        bind_group_desc.entryCount = entries.size();
        bind_group_desc.entries = entries.data();
        m_bind_group = m_device.CreateBindGroup(&bind_group_desc);

        // Every rect is a quad of 4 vertices drawn as two triangles.
        std::vector<uint32_t> indices;
        indices.reserve(capacity * 6);
        for (uint32_t i = 0; i < capacity; ++i)
        {
            const uint32_t base = i * 4;
            indices.insert(indices.end(), { base, base + 1, base + 2, base + 2, base + 1, base + 3 });
        }

        wgpu::BufferDescriptor index_desc{};
        index_desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        index_desc.size = indices.size() * sizeof(uint32_t);
        m_index_buffer = m_device.CreateBuffer(&index_desc);
        m_queue.WriteBuffer(m_index_buffer, 0, indices.data(), indices.size() * sizeof(uint32_t));

        m_rect_capacity = capacity;
    }

    void WebGpuRenderer::clear(Color c)
    {
        m_rects.clear();
        m_clear_color = {c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0};
    }

    void WebGpuRenderer::fill_rect(const Rect& rect, Color c)
    {
        if (!m_ready) return;
        m_rects.push_back({rect, c});
    }

    void WebGpuRenderer::present()
    {
        if (!m_ready) return;

        wgpu::SurfaceTexture surface_texture{};
        m_surface.GetCurrentTexture(&surface_texture);
        if (surface_texture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
            surface_texture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)
        {
            return;
        }

        const size_t rect_count = m_rects.size();
        ensure_capacity(rect_count);
        if (rect_count > 0)
        {
            m_queue.WriteBuffer(m_rect_buffer, 0, m_rects.data(), m_rects.size() * sizeof(RectData));
        }

        wgpu::RenderPassColorAttachment attachment{};
        attachment.view = surface_texture.texture.CreateView();
        attachment.loadOp = wgpu::LoadOp::Clear;
        attachment.storeOp = wgpu::StoreOp::Store;
        attachment.clearValue = m_clear_color;

        wgpu::RenderPassDescriptor pass_desc{};
        pass_desc.colorAttachmentCount = 1;
        pass_desc.colorAttachments = &attachment;

        wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&pass_desc);
        if (rect_count > 0)
        {
            pass.SetPipeline(m_pipeline);
            pass.SetBindGroup(0, m_bind_group);
            pass.SetIndexBuffer(m_index_buffer, wgpu::IndexFormat::Uint32);
            pass.DrawIndexed(static_cast<uint32_t>(rect_count * 6));
        }
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        m_queue.Submit(1, &commands);
    }
}
