#include <utility>

#include "AppGL/OpenGL/glfw/App.h"
#include "AppGL/shader/Shader.h"

#include "AppGL/shader/ComputeShader.h"

#include <memory>

class MyApp : public AppGL::App
{
public:
        explicit MyApp(AppGL::AppOptions options): AppGL::App(std::move(options)) {}

private:
        std::shared_ptr<ComputeShader> computeShader;
        int texBinding = 0;

        FILE* ffmpeg{};
        std::vector<uint8_t> frameBuffer{};

        float frames = 0.0;

        void onStart() override
        {
                const char computeCode[] = {
#embed "../src/shadersource/mandelbulb.comp"
                        , '\0' };

                const char vertCode[] = {
#embed "../src/shadersource/vertex.vert"
                        , '\0' };

                const char fragCode[] = {
#embed "../src/shadersource/fragment.frag"
                        , '\0' };

                computeShader = std::make_shared<ComputeShader>(computeCode, vertCode, fragCode);
                computeShader->localSizeX = 8;
                computeShader->localSizeY = 4;

                computeShader->addUniform("tex", texBinding);
                computeShader->addUniform("resolution", glm::vec2(0.0));
                computeShader->addUniform("frames", 0.0f);

                const uint32_t windowWidth = this->window.getWidth(), windowHeight = this->window.getHeight();

                // FFMPEG Setup.
                const std::string command =
                    "ffmpeg -y "
                    "-f rawvideo -pixel_format rgb24 -video_size " + std::to_string(windowWidth) + "x" + std::to_string(windowHeight) + " "
                    "-framerate 60 "        // treat each input frame as 1/60 s
                    "-i - "
                    "-c:v libx264 -preset fast -crf 18 -pix_fmt yuv420p "
                    "output.mp4";
                ffmpeg = popen(command.c_str(), "w");
                frameBuffer = std::vector<uint8_t>(this->window.getWidth() * this->window.getHeight() * 3);
        }

        double lastTime = AppGL::Window::getTime();
        int frameCount = 0;

        void onUpdate() override
        {
                frames++;
                frameCount++;
                double currentTime = AppGL::Window::getTime();
                if (currentTime - lastTime >= 1.0) { // every 1 second
                        std::cout << "FPS: " << frameCount << std::endl;
                        frameCount = 0;
                        lastTime = currentTime;
                }

                const glm::ivec2 resolution = this->window.getResolution();

                computeShader->setUniform("tex", texBinding);
                computeShader->setUniform("resolution", (glm::vec2)resolution);
                computeShader->setUniform("frames", frames);

                // Draw
                computeShader->drawFullScreenQuad(resolution.x, resolution.y, texBinding);

                // To FFMPEG
                this->window.writeFrameBufferToFile(frameBuffer, ffmpeg);
        }

        void onDestroy() override
        {
                pclose(ffmpeg);
        }
};

AppGL::App* AppGL::InitApp()
{
        AppOptions options;
        options.width = 1920;
        options.height = 1080;
        options.title = "Fractal-SDFs";
        
        AppGL::App* app = new MyApp(options);
        return app;
}
