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

struct Vertex
{
    @location(0) position: vec2f,
    @location(1) color: vec4f,
};

struct VsOut
{
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
};

@vertex
fn vs_main(v: Vertex) -> VsOut
{
    let ndc = vec2f(v.position.x / globals.size.x * 2.0 - 1.0, 1.0 - v.position.y / globals.size.y * 2.0);

    var out: VsOut;
    out.position = vec4f(ndc, 0.0, 1.0);
    out.color = v.color;
    return out;
}

@fragment
fn fs_main(in: VsOut) -> @location(0) vec4f
{
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

        std::array<wgpu::VertexAttribute, 2> attributes{};
        attributes[0].format = wgpu::VertexFormat::Float32x2;
        attributes[0].offset = offsetof(Vertex, x);
        attributes[0].shaderLocation = 0;
        attributes[1].format = wgpu::VertexFormat::Unorm8x4;
        attributes[1].offset = offsetof(Vertex, color);
        attributes[1].shaderLocation = 1;

        wgpu::VertexBufferLayout vertex_layout{};
        vertex_layout.stepMode = wgpu::VertexStepMode::Vertex;
        vertex_layout.arrayStride = sizeof(Vertex);
        vertex_layout.attributeCount = attributes.size();
        vertex_layout.attributes = attributes.data();

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
        pipeline_desc.vertex.bufferCount = 1;
        pipeline_desc.vertex.buffers = &vertex_layout;
        pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipeline_desc.fragment = &fragment;
        m_pipeline = m_device.CreateRenderPipeline(&pipeline_desc);

        wgpu::BufferDescriptor uniform_desc{};
        uniform_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        uniform_desc.size = 16;
        m_uniform_buffer = m_device.CreateBuffer(&uniform_desc);

        const std::array<float, 4> globals{static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 0.0f};
        m_queue.WriteBuffer(m_uniform_buffer, 0, globals.data(), sizeof(globals));

        wgpu::BindGroupEntry entry{};
        entry.binding = 0;
        entry.buffer = m_uniform_buffer;
        entry.size = sizeof(globals);

        wgpu::BindGroupDescriptor bind_group_desc{};
        bind_group_desc.layout = m_pipeline.GetBindGroupLayout(0);
        bind_group_desc.entryCount = 1;
        bind_group_desc.entries = &entry;
        m_bind_group = m_device.CreateBindGroup(&bind_group_desc);

        ensure_capacity(INITIAL_RECT_CAPACITY);
        m_ready = true;
    }

    void WebGpuRenderer::ensure_capacity(size_t rect_count)
    {
        if (rect_count <= m_rect_capacity) return;

        size_t capacity = std::max(m_rect_capacity, INITIAL_RECT_CAPACITY);
        while (capacity < rect_count) capacity *= 2;

        wgpu::BufferDescriptor vertex_desc{};
        vertex_desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertex_desc.size = capacity * 4 * sizeof(Vertex);
        m_vertex_buffer = m_device.CreateBuffer(&vertex_desc);

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
        m_vertices.clear();
        m_clear_color = {c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0};
    }

    void WebGpuRenderer::fill_rect(const Rect& rect, Color c)
    {
        if (!m_ready) return;
        const float x0 = rect.x, y0 = rect.y;
        const float x1 = rect.x + rect.w, y1 = rect.y + rect.h;
        m_vertices.push_back({ x0, y0, c });
        m_vertices.push_back({ x1, y0, c });
        m_vertices.push_back({ x0, y1, c });
        m_vertices.push_back({ x1, y1, c });
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

        const size_t rect_count = m_vertices.size() / 4;
        ensure_capacity(rect_count);
        if (rect_count > 0)
        {
            m_queue.WriteBuffer(m_vertex_buffer, 0, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
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
            pass.SetVertexBuffer(0, m_vertex_buffer);
            pass.SetIndexBuffer(m_index_buffer, wgpu::IndexFormat::Uint32);
            pass.DrawIndexed(static_cast<uint32_t>(rect_count * 6));
        }
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        m_queue.Submit(1, &commands);
    }
}
