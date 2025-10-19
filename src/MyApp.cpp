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

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        float frames = 0.0f;

        int texBinding = 0;

        FILE* ffmpeg{};
        std::vector<uint8_t> frame{};

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
                computeShader->addUniform("time", 0.0f);

                const uint32_t windowWidth = this->window.getWidth(), windowHeight = this->window.getHeight();

                // FFMPEG Setup.
                const std::string command = "ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size "
                                    + std::to_string(windowWidth) + "x" + std::to_string(windowHeight)
                                    + " -r 60 -i - -c:v libx264 -preset fast -crf 18 output.mp4";
                ffmpeg = popen(command.c_str(), "w");
                frame = std::vector<uint8_t>(this->window.getWidth() * this->window.getHeight() * 3);
        }

        void onUpdate() override
        {
                float currentFrame = this->window.getTime();
                deltaTime = currentFrame - lastFrame;
                lastFrame = currentFrame;
                if (frames > 500) {
                        std::cout << "FPS: " << 1 / deltaTime << std::endl;
                        frames = 0;
                } else frames++;

                glm::ivec2 resolution = this->window.getResolution();

                computeShader->setUniform("tex", texBinding);
                computeShader->setUniform("resolution", (glm::vec2)resolution);
                computeShader->setUniform("time", currentFrame);

                // Draw
                computeShader->drawFullScreenQuad(resolution.x, resolution.y, texBinding);

                // To FFMPEG
                this->window.writeFrameBufferToFile(frame, ffmpeg);
        }

        void onDestroy() override
        {
                pclose(ffmpeg);
        }
};

AppGL::App* AppGL::InitApp()
{
        AppOptions options;
        options.width = 1000;
        options.height = 1000;
        options.title = "Fractal-SDFs";
        
        AppGL::App* app = new MyApp(options);
        return app;
}