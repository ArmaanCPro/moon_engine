#pragma once

#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
#include <vulkan/vulkan.hpp> // consider transitioning to c++ module import

#include <vma/vk_mem_alloc.h>
#include <VkBootstrap.h>

struct allocated_image
{
    vk::Image image;
    vk::ImageView view;
    VmaAllocation allocation;
    vk::Extent3D extent;
    vk::Format format;
};
