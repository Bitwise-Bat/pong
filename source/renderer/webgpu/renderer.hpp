#pragma once
#include <vector>
#include <webgpu/webgpu_cpp.h>

#include "renderer/renderer.hpp"

namespace pong
{
    // Device setup is asynchronous in the browser, so drawing calls are ignored until ready().
    class WebGpuRenderer : public Renderer
    {
    public:
        WebGpuRenderer(uint32_t width, uint32_t height);

        void clear(Color c) override;
        using Renderer::fill_rect;
        void fill_rect(const Rect& rect, Color c, int32_t texture, const Rect& uv) override;
        void upload_texture(int32_t texture, uint32_t x, uint32_t y,
                            uint32_t width, uint32_t height, std::span<const uint8_t> rgba) override;
        void present();

        [[nodiscard]] bool ready() const { return m_ready; }

    private:
        struct alignas(16) RectData
        {
            Rect rect;
            Rect uv;
            Color color;
            int32_t texture;
        };

        struct PendingUpload
        {
            int32_t texture;
            uint32_t x, y, width, height;
            std::vector<uint8_t> rgba;
        };

        void write_texture(const PendingUpload& upload);
        void request_device(wgpu::Adapter adapter);
        void init_resources();
        void ensure_capacity(size_t rect_count);

    private:
        uint32_t m_width{};
        uint32_t m_height{};
        bool m_ready{};

        wgpu::Instance m_instance{};
        wgpu::Surface m_surface{};
        wgpu::Device m_device{};
        wgpu::Queue m_queue{};
        wgpu::TextureFormat m_format{};

        wgpu::RenderPipeline m_pipeline{};
        wgpu::Buffer m_uniform_buffer{};
        wgpu::Texture m_textures{};
        wgpu::TextureView m_textures_view{};
        wgpu::Sampler m_sampler{};
        wgpu::BindGroup m_bind_group{};
        wgpu::Buffer m_rect_buffer{};
        wgpu::Buffer m_index_buffer{};
        size_t m_rect_capacity{};

        std::vector<RectData> m_rects{};
        std::vector<PendingUpload> m_pending_uploads{};
        wgpu::Color m_clear_color{};
    };
}
