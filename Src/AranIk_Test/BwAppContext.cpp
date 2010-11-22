#include "BwPch.h"
#include "BwAppContext.h"

const float BwAppContext::massMapDeviation = 0.05f;

BwAppContext::BwAppContext()
  : bipedComPos(50)
  , massMapData(new unsigned char[massMapResolution*2 * massMapResolution*2 * 4])
  , activeCam(0)
  , activeLight(0)
  , frames(0)
  , rb_history(MAX_SIMULATION_FRAMES)
{
  massMap.resize(massMapResolution*2);
  for (int i = 0; i < massMapResolution*2; ++i) {
    massMap[i].resize(massMapResolution*2);
  }
  sprintf(frameStr, "%d", frames);
  // OpenGL context is not available at this time.
}

BwAppContext::~BwAppContext()
{
  foreach (ArnIkSolver* ikSolver, ikSolvers) {
    delete ikSolver;
  }
  ikSolvers.clear();
  glDeleteTextures(1, &massMapTex);
  delete [] massMapData;
}
