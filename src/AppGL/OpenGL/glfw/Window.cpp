#include "Window.h"

#include <cstring>
#include <stdexcept>

#include <glad/glad.h>
#include "GLFW/glfw3.h"

namespace AppGL
{
        Window::Window(const AppOptions& options): width(options.width), height(options.height)
        {
                glfwInit();

                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

                if (options.pixelFormatAttributes.has_value()) {
                        for (const auto& [flag, value] : options.pixelFormatAttributes.value())
                                glfwWindowHint(flag, value);
                }

                window = glfwCreateWindow(
                        (int)width, (int)height, options.title,
                        nullptr, nullptr
                );

                if (!window) {
                        glfwTerminate();
                        throw std::runtime_error("Failed to create GLFW window.");
                }

                if (options.position.has_value())
                        glfwSetWindowPos(window, options.position.value().first, options.position.value().second);

                glfwMakeContextCurrent(window);
                glfwSetWindowUserPointer(window, this);
                glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

                glfwSwapInterval(0);
        }

        void Window::updateWindowSize() const
        {
                glViewport(0, 0, width, height);
        }

        float Window::getTime()
        {
                return static_cast<float>(glfwGetTime());
        }

        glm::vec2 Window::getResolution() const
        {
                int width, height;
                glfwGetWindowSize(window, &width, &height);
                return { width, height };
        }

        void Window::setResolution(const glm::uvec2 newResolution)
        {
                width = newResolution.x;
                height = newResolution.y;
                updateWindowSize();
        }

        void  Window::setWidth(const uint32_t newWidth)
        {
                width = newWidth;
                updateWindowSize();
        }

        void  Window::setHeight(const uint32_t newHeight)
        {
                height = newHeight;
                updateWindowSize();
        }

        void Window::framebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight)
        {
                auto* thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                thisWindow->setResolution(glm::uvec2(newWidth, newHeight));

                glViewport(0, 0, newWidth, newHeight);
        }

        void Window::writeFrameBufferToFile(std::vector<uint8_t>& frame, FILE* file) const
        {
                glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, frame.data());

                int rowSize = width * 3;
                std::vector<uint8_t> tmp(rowSize);
                for (int y = 0; y < height / 2; ++y)
                {
                        uint8_t* rowTop = frame.data() + y * rowSize;
                        uint8_t* rowBot = frame.data() + (height - 1 - y) * rowSize;
                        memcpy(tmp.data(), rowTop, rowSize);
                        memcpy(rowTop, rowBot, rowSize);
                        memcpy(rowBot, tmp.data(), rowSize);
                }

                fwrite(frame.data(), 1, frame.size(), file);
        }
}

