#pragma once

#include "moon/core/core.h"
#include "handle.h"
#include "core/log.h"

#include <vector>

namespace moon
{
    // Pool is only implemented from and used in the backend
    template <typename ObjectType, typename ImplObjectType>
    class Pool
    {
        static constexpr uint32_t s_ListEndSentinal = std::numeric_limits<uint32_t>::max();
        struct PoolEntry
        {
            explicit PoolEntry(ImplObjectType&& obj) : obj_(std::move(obj)) {}
            ImplObjectType obj_;
            uint32_t gen_ = 1;
            uint32_t next_free_ = s_ListEndSentinal;
        };
        uint32_t m_free_list_head = s_ListEndSentinal;
        uint32_t m_num_objects = 0;

    public:
        std::vector<PoolEntry> m_objects;

        handle<ObjectType> create(ImplObjectType&& obj)
        {
            uint32_t index;
            if (m_free_list_head != s_ListEndSentinal)
            {
                index = m_free_list_head;
                m_free_list_head = m_objects[index].next_free_;
                m_objects[index].obj_ = std::move(obj);
            }
            else
            {
                index = (uint32_t)m_objects.size();
                m_objects.emplace_back(obj);
            }
            m_num_objects++;
            return handle<ObjectType>(index, m_objects[index].gen_);
        }

        void destroy(handle<ObjectType> handle_to_destroy)
        {
            if (!handle_to_destroy)
                return;
            MOON_CORE_ASSERT(m_num_objects > 0, "Trying to destroy a handle from an empty pool!");
            const uint32_t index = handle_to_destroy.index();
            MOON_CORE_ASSERT(index < m_objects.size(), "Trying to destroy a handle with invalid index out of bounds!");
            MOON_CORE_ASSERT(m_objects[index].gen_ == handle_to_destroy.gen(), "Trying to destroy a handle with invalid generation! (likely double deletion)");
            m_objects[index].obj_ = ImplObjectType();
            ++m_objects[index].gen_;
            m_objects[index].next_free_ = m_free_list_head;
            m_objects[index].next_free_ = m_free_list_head;
            m_free_list_head = index;
            m_num_objects--;
        }

        const ImplObjectType* get(handle<ObjectType> handle_to_get) const
        {
            if (!handle_to_get)
                return nullptr;
            const uint32_t index = handle_to_get.index();
            MOON_CORE_ASSERT(index < m_objects.size(), "Trying to get a handle with invalid index out of bounds!");
            MOON_CORE_ASSERT(m_objects[index].gen_ == handle_to_get.gen(), "Trying to get a deleted object!");
            return &m_objects[index].obj_;
        }

        ImplObjectType* get(handle<ObjectType> handle_to_get)
        {
            if (!handle_to_get)
                return nullptr;
            const uint32_t index = handle_to_get.index();
            MOON_CORE_ASSERT(index < m_objects.size(), "Trying to get a handle with invalid index out of bounds!");
            MOON_CORE_ASSERT(m_objects[index].gen_ == handle_to_get.gen(), "Trying to get a deleted object!");
            return &m_objects[index].obj_;
        }

        handle<ObjectType> get_handle(uint32_t index) const
        {
            MOON_CORE_ASSERT(index < m_objects.size(), "Trying to get a handle with invalid index out of bounds!");
            if (index >= m_objects.size())
                return {};
            return handle<ObjectType>(index, m_objects[index].gen_);
        }

        handle<ObjectType> find_handle(const ImplObjectType* obj)
        {
            if (!obj)
                return {};
            for (size_t index = 0; index != m_objects.size(); ++index)
            {
                if (&m_objects[index].obj_ == *obj)
                    return handle<ObjectType>((uint32_t)index, m_objects[index].gen_);
            }
            return {};
        }

        void clear()
        {
            m_objects.clear();
            m_free_list_head = s_ListEndSentinal;
            m_num_objects = 0;
        }

        uint32_t num_objects() const noexcept { return m_num_objects; }
    };
}
