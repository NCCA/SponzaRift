#ifndef OCULUSINTERFACE_H__
#define OCULUSINTERFACE_H__
#include <ngl/Mat4.h>
#include <OVR.h>
#include <OVR_CAPI_GL.h>


class OculusInterface
{
  public :
    /// @brief to return the instance of the class
    /// @returns the constructed class
    static OculusInterface* instance();
    void initOculus(float _devicePixelAspect);


    inline void debugOn(){m_debug=true;}
    inline void debugOff(){m_debug=false;}
    inline void turnOffWarning(){m_warningOff=true;}
    ngl::Vec3 getLeftEyeOffset() const ;
    ngl::Vec3 getRightEyeOffset() const ;


    void oculusDebug() const;
    void releaseOculus();
    void oculusDisplayWarning();
    void oculusPoseState();
    void beginFrame();
    void endFrame();
    void setLeftEye();
    void setRightEye();
    void setDevicePixelAspect(float _f){m_devicePixelAspect=_f;}
    void disableWarningMessage();
    int getWidth() { return m_hmd->Resolution.w;}
    int getHeight() { return m_hmd->Resolution.h;}
    ngl::Mat4 getPerspectiveMatrix(int _eye);
    ngl::Mat4 getViewMatrix(int _eye);

  private :
    // singleton
    OculusInterface();
    ~OculusInterface(){;}
    OculusInterface(const OculusInterface &_t){;}
    // create the FBO render target.
    void createRenderTarget();
    // fill GL texture buffer
    void createOVRTextureBuffers();
    // fill OVR GL config data
    void createOVRGLConfig();
    /// @brief the instance of the class
    static OculusInterface* m_pinstance;
    /// @brief debug flag
    bool m_debug;
    // Oculus head mounted display pointer
     ovrHmd m_hmd;
    // turn off the warning
    bool m_warningOff;
    // size of the frame buffer for rendering
    int m_fbWidth;
    int m_fbHeight;
    // id of the FBO
    GLuint m_fbo;
    // texture buffer for FBO
    GLuint m_fboTex;
    // depth buffer for FBO
    GLuint m_fboDepth;
    // width of actual texture (next pow 2)
    GLuint m_fboTexWidth;
    // height of actual texture (next pow 2)
    GLuint m_fboTexHeight;
     ovrSizei m_eyeres[2];
     ovrEyeRenderDesc m_eyeRdesc[2];
     ovrGLTexture m_fbTextureIDOVR[2];
    // config structure
    union ovrGLConfig m_glcfg;
    ovrPosef m_pose[2];
    GLuint m_windowWidth;
    GLuint m_windowHeight;
    GLfloat m_devicePixelAspect;


};



#endif
