#include "stdafx.h"
#include "MainMenuRenderPipeline.h"

#include <Vorb/graphics/TextureCache.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/io/FileOps.h>

#include "AwesomiumRenderStage.h"
#include "Errors.h"
#include "GameManager.h"
#include "HdrRenderStage.h"
#include "Options.h"
#include "SkyboxRenderStage.h"
#include "SoaState.h"
#include "SpaceSystemRenderStage.h"
#include "soaUtils.h"

MainMenuRenderPipeline::MainMenuRenderPipeline() {
    // Empty
}


MainMenuRenderPipeline::~MainMenuRenderPipeline() {
    destroy(true);
}

void MainMenuRenderPipeline::init(const SoaState* soaState, const ui32v4& viewport,
                                  Camera* camera,
                                  IAwesomiumInterface* awesomiumInterface,
                                  SpaceSystem* spaceSystem,
                                  const MainMenuSystemViewer* systemViewer) {
    // Set the viewport
    m_viewport = viewport;

    // Check to make sure we don't double init
    if (m_isInitialized) {
        pError("Reinitializing MainMenuRenderPipeline without first calling destroy()!");
        return;
    } else {
        m_isInitialized = true;
    }

    // Construct framebuffer
    m_hdrFrameBuffer = new vg::GLRenderTarget(m_viewport.z, m_viewport.w);
    m_hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, graphicsOptions.msaa).initDepth();
    if (graphicsOptions.msaa > 0) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }

    // Make swap chain
    m_swapChain = new vg::RTSwapChain<2>(m_viewport.z, m_viewport.w);
    m_swapChain->init(vg::TextureInternalFormat::RGBA8);
    m_quad.init();

    // Helpful macro to reduce code size
#define ADD_STAGE(type, ...) static_cast<type*>(addStage(std::make_shared<type>(__VA_ARGS__)))

    // Init render stages
    m_skyboxRenderStage = ADD_STAGE(SkyboxRenderStage, camera, &soaState->texturePackIom);
    m_awesomiumRenderStage = ADD_STAGE(AwesomiumRenderStage, awesomiumInterface);
    m_hdrRenderStage = ADD_STAGE(HdrRenderStage, &m_quad, camera);
    // TODO(Ben): Use texture pack iomanager
    m_spaceSystemRenderStage = ADD_STAGE(SpaceSystemRenderStage, ui32v2(m_viewport.z, m_viewport.w),
                                                          spaceSystem, nullptr, systemViewer, camera, nullptr,
                                                          GameManager::textureCache->addTexture("Textures/selector.png").id);
}

void MainMenuRenderPipeline::render() {
    
    // Bind the FBO
    m_hdrFrameBuffer->use();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    m_skyboxRenderStage->render();
    m_spaceSystemRenderStage->setShowAR(m_showAR);
    m_spaceSystemRenderStage->render();

    // Post processing
    m_swapChain->reset(0, m_hdrFrameBuffer, graphicsOptions.msaa > 0, false);

    // TODO: More Effects?

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(m_hdrFrameBuffer->getTextureTarget(), m_hdrFrameBuffer->getTextureDepthID());
    m_hdrRenderStage->render();

    // Render stuff that doesn't want HDR
    glBlendFunc(GL_ONE, GL_ONE);
    m_spaceSystemRenderStage->renderStarGlows();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_showUI) m_awesomiumRenderStage->render();

    if (m_shouldScreenshot) dumpScreenshot();

    // Check for errors, just in case
    checkGlError("MainMenuRenderPipeline::render()");
}

void MainMenuRenderPipeline::destroy(bool shouldDisposeStages) {
    if (!m_isInitialized) return;
    RenderPipeline::destroy(shouldDisposeStages);

    if (m_swapChain) {
        m_swapChain->dispose();
        delete m_swapChain;
        m_swapChain = nullptr;
    }

    m_quad.dispose();

    m_isInitialized = false;
}

void MainMenuRenderPipeline::dumpScreenshot() {
    // Make screenshots directory
    vio::buildDirectoryTree("Screenshots");
    // Take screenshot
    dumpFramebufferImage("Screenshots/", m_viewport);
    m_shouldScreenshot = false;
}
