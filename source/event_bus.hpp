#pragma once
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace pong
{
    class EventBus
    {
    public:
        class Subscription
        {
        public:
            Subscription() = default;
            Subscription(EventBus* bus, std::type_index type, std::size_t id)
                : m_bus(bus), m_type(type), m_id(id) {}
            ~Subscription() { reset(); }

            Subscription(Subscription&& o) noexcept { *this = std::move(o); }
            Subscription& operator=(Subscription&& o) noexcept
            {
                reset();
                m_bus = o.m_bus; m_type = o.m_type; m_id = o.m_id;
                o.m_bus = nullptr;
                return *this;
            }
            Subscription(const Subscription&) = delete;
            Subscription& operator=(const Subscription&) = delete;

            void reset() { if (m_bus) m_bus->unsubscribe(m_type, m_id); m_bus = nullptr; }

        private:
            EventBus*       m_bus = nullptr;
            std::type_index m_type = typeid(void);
            std::size_t     m_id = 0;
        };

        template <typename E, typename F>
        [[nodiscard]] Subscription subscribe(F&& handler)
        {
            const std::size_t id = ++m_nextId;
            m_handlers[typeid(E)].push_back({ id, [h = std::forward<F>(handler)](const void* e)
            {
                h(*static_cast<const E*>(e));
            }});
            return Subscription(this, typeid(E), id);
        }

        template <typename E>
        void publish(E event)
        {
            m_queue.push_back([this, e = std::move(event)] { dispatch(typeid(E), &e); });
        }

        void flush()
        {
            auto pending = std::move(m_queue);
            m_queue.clear();
            for (auto& fn : pending) fn();
        }

    private:
        struct Handler { std::size_t id; std::function<void(const void*)> fn; };

        void dispatch(std::type_index type, const void* e)
        {
            auto it = m_handlers.find(type);
            if (it == m_handlers.end()) return;
            auto handlers = it->second;
            for (auto& h : handlers) h.fn(e);
        }

        void unsubscribe(std::type_index type, std::size_t id)
        {
            auto& v = m_handlers[type];
            std::erase_if(v, [id](const Handler& h) { return h.id == id; });
        }

        std::unordered_map<std::type_index, std::vector<Handler>> m_handlers;
        std::vector<std::function<void()>> m_queue;
        std::size_t m_nextId = 0;
    };
}
