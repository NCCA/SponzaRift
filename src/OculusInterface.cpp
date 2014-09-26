#include "OculusInterface.h"
#include <iostream>
#include <ngl/Util.h>
#include <ngl/Transformation.h>
#include <ngl/Quaternion.h>
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

OculusInterface* OculusInterface::m_pinstance = 0;// initialize pointer
//ovrHmd OculusInterface::m_hmd;
//ovrSizei OculusInterface::m_eyeres[2];
//ovrEyeRenderDesc OculusInterface::m_eyeRdesc[2];
//ovrGLTexture OculusInterface::m_fbTextureIDOVR[2];
//union ovrGLConfig OculusInterface::m_glcfg;

OculusInterface* OculusInterface::instance()
{
  if (m_pinstance == 0)  // is it the first call?
  {
    m_pinstance = new OculusInterface; // create sole instance
  }
  return m_pinstance; // address of sole instance
}


OculusInterface::OculusInterface()
{
  m_debug=true;
  m_warningOff=false;
  ovr_Initialize();
}
void OculusInterface::initOculus(float _devicePixelAspect)
{
  m_devicePixelAspect=_devicePixelAspect;
  std::cout<<"setting device aspect "<<m_devicePixelAspect<<"\n";
  m_hmd = ovrHmd_Create(0);
  if (!m_hmd)
  {
    std::cerr<<"Unable to create HMD: "<< ovrHmd_GetLastError(NULL)<<std::endl;
    std::cerr<<"Attempting to run without HMD\n";
    // If we didn't detect an Hmd, create a simulated one for debugging.
    m_hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
    if (!m_hmd)
    {   // Failed Hmd creation.
      exit(EXIT_FAILURE);
    }
  }
  m_windowWidth=m_hmd->Resolution.w;
  m_windowHeight=m_hmd->Resolution.h;

  oculusDebug();
  // Start the sensor which provides the Rift’s pose and motion.
  ovrHmd_ConfigureTracking(m_hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
  // let's fill in some info about oculus
  m_eyeres[0] = ovrHmd_GetFovTextureSize(m_hmd, ovrEye_Left, m_hmd->DefaultEyeFov[0], 1.0);
  m_eyeres[1] = ovrHmd_GetFovTextureSize(m_hmd, ovrEye_Right, m_hmd->DefaultEyeFov[1], 1.0);

	/* and create a single render target texture to encompass both eyes */
	m_fbWidth = m_eyeres[0].w + m_eyeres[1].w;
	m_fbHeight = m_eyeres[0].h > m_eyeres[1].h ? m_eyeres[0].h : m_eyeres[1].h;
	createRenderTarget();
	createOVRGLConfig();
	createOVRTextureBuffers();
	/* enable low-persistence display and dynamic prediction for lattency compensation */
	ovrHmd_SetEnabledCaps(m_hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

	/* configure SDK-rendering and enable chromatic abberation correction, vignetting, and
	 * timewrap, which shifts the image before drawing to counter any lattency between the call
	 * to ovrHmd_GetEyePose and ovrHmd_EndFrame.
	 */
	unsigned int dcaps = ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp |
		ovrDistortionCap_Overdrive;
	if(!ovrHmd_ConfigureRendering(m_hmd, &m_glcfg.Config, dcaps, m_hmd->DefaultEyeFov, m_eyeRdesc))
	{
		fprintf(stderr, "failed to configure distortion renderer\n");
	}


}


void OculusInterface::oculusDebug() const
{
  if(m_debug == true)
  {
    std::cerr<<"Type "<<m_hmd->Type<<"\n";
    std::cerr<<"ProductName "<<m_hmd->ProductName<<"\n";
    std::cerr<<"Manufacturer "<<m_hmd->Manufacturer<<"\n";
    std::cerr<<"VendorId "<<m_hmd->VendorId<<"\n";
    std::cerr<<"ProductId "<<m_hmd->ProductId<<"\n";
    std::cerr<<"SerialNumber "<<m_hmd->SerialNumber<<"\n";
    std::cerr<<"FirmwareMajor "<<m_hmd->FirmwareMajor<<"\n";
    std::cerr<<"FirmwareMinor "<<m_hmd->FirmwareMinor<<"\n";
    std::cerr<<"CameraFrustumHFovInRadians "<<m_hmd->CameraFrustumHFovInRadians<<"\n";
    std::cerr<<"CameraFrustumVFovInRadians "<<m_hmd->CameraFrustumVFovInRadians<<"\n";
    std::cerr<<"CameraFrustumNearZInMeters "<<m_hmd->CameraFrustumNearZInMeters<<"\n";
    std::cerr<<"CameraFrustumFarZInMeters "<<m_hmd->CameraFrustumFarZInMeters<<"\n";

    std::cerr<<"HmdCaps "<<m_hmd->HmdCaps<<"\n";
    std::cerr<<"TrackingCaps "<<m_hmd->TrackingCaps<<"\n";
    std::cerr<<"DistortionCaps "<<m_hmd->DistortionCaps<<"\n";

    std::cerr<<"Resolution "<<m_hmd->Resolution.w<<" "<<m_hmd->Resolution.h<<"\n";

    std::cerr<<"WindowsPos "<<m_hmd->WindowsPos.x<<" "<<m_hmd->WindowsPos.y<<"\n";

    std::cerr<<"DefaultEyeFov Up Down"<<m_hmd->DefaultEyeFov[0].UpTan<<" "<<m_hmd->DefaultEyeFov[0].DownTan<<"\n";
    std::cerr<<"DefaultEyeFov Left Right"<<m_hmd->DefaultEyeFov[0].LeftTan<<" "<<m_hmd->DefaultEyeFov[0].RightTan<<"\n";

    std::cerr<<"MaxEyeFov Up Down"<<m_hmd->DefaultEyeFov[1].UpTan<<" "<<m_hmd->DefaultEyeFov[1].DownTan<<"\n";
    std::cerr<<"MaxEyeFov Left Right"<<m_hmd->DefaultEyeFov[1].LeftTan<<" "<<m_hmd->DefaultEyeFov[1].RightTan<<"\n";


    std::cerr<<"DisplayDeviceName "<<m_hmd->DisplayDeviceName<<"\n";
    std::cerr<<"DisplayId "<<m_hmd->DisplayId<<"\n";
  }
}

void OculusInterface::releaseOculus()
{
 ovrHmd_Destroy(m_hmd);
 // shutdown OVR SDK
 ovr_Shutdown();
}


void OculusInterface::oculusDisplayWarning()
{
  // Health and Safety Warning display state.
  ovrHSWDisplayState hswDisplayState;
  ovrHmd_GetHSWDisplayState(m_hmd, &hswDisplayState);
  if (hswDisplayState.Displayed)
  {
  // Dismiss the warning if the user pressed the appropriate key or if the user
  // is tapping the side of the HMD.
  // If the user has requested to dismiss the warning via keyboard or controller input...
  if (m_warningOff)
    ovrHmd_DismissHSWDisplay(m_hmd);
  else
  {
  // Detect a moderate tap on the side of the HMD.
  ovrTrackingState ts = ovrHmd_GetTrackingState(m_hmd, ovr_GetTimeInSeconds());
  if (ts.StatusFlags & ovrStatus_OrientationTracked)
  {
  const OVR::Vector3f v(ts.RawSensorData.Accelerometer.x,
  ts.RawSensorData.Accelerometer.y,
  ts.RawSensorData.Accelerometer.z);
  // Arbitrary value and representing moderate tap on the side of the DK2 Rift.
  if (v.LengthSq() > 250.f)
  ovrHmd_DismissHSWDisplay(m_hmd);
  }
  }
}
}


void OculusInterface::oculusPoseState()
{
  ovrTrackingState ts = ovrHmd_GetTrackingState(m_hmd, ovr_GetTimeInSeconds());
  if (ts.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
  {
    ovrPoseStatef pose = ts.HeadPose;
    std::cout<<"--------------------------------------------------------------\n";
    std::cout<<"Time "<<pose.TimeInSeconds<<"\n";
    std::cout<<"Orientation Quat <<"<< pose.ThePose.Orientation.x <<" "
              << pose.ThePose.Orientation.y <<" "
              << pose.ThePose.Orientation.z <<" "
              << pose.ThePose.Orientation.w <<"\n";
    std::cout << "Angular Velocity "<< pose.AngularVelocity.x <<" "
                 << pose.AngularVelocity.y <<" "
                 << pose.AngularVelocity.z <<"\n";
    std::cout << "Linear Velocity "<< pose.LinearVelocity.x <<" "
                 << pose.LinearVelocity.y <<" "
                 << pose.LinearVelocity.z <<"\n";
    std::cout << "AngularAcceleration Velocity "<< pose.AngularAcceleration.x <<" "
                 << pose.AngularAcceleration.y <<" "
                 << pose.AngularAcceleration.z <<"\n";

    std::cout << "LinearAcceleration Velocity "<< pose.LinearAcceleration.x <<" "
                 << pose.LinearAcceleration.y <<" "
                 << pose.LinearAcceleration.z <<"\n";
    std::cout<<"--------------------------------------------------------------\n";


  }
}

void OculusInterface::createRenderTarget()
{

		if(!m_fbo)
		{
			std::cout<< "Creating FBO \n";
			/* if fbo does not exist, then nothing does... create every opengl object */
			glGenFramebuffers(1, &m_fbo);
			glGenTextures(1, &m_fboTex);
			glGenRenderbuffers(1, &m_fboDepth);

			glBindTexture(GL_TEXTURE_2D, m_fboTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		std::cout<<"Creating Texture objects\n";
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		/* calculate the next power of two in both dimensions and use that as a texture size */
		m_fboTexWidth = ngl::nextPow2(m_fbWidth);
		m_fboTexHeight = ngl::nextPow2(m_fbHeight);

		/* create and attach the texture that will be used as a color buffer */
		glBindTexture(GL_TEXTURE_2D, m_fboTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_fboTexWidth, m_fboTexHeight, 0,	GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTex, 0);

		/* create and attach the renderbuffer that will serve as our z-buffer */
		glBindRenderbuffer(GL_RENDERBUFFER, m_fboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_fboTexWidth, m_fboTexHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fboDepth);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr<<"incomplete framebuffer!\n";
		}
		std::cout<<"Render target created "<< m_fbWidth << " " << m_fbHeight <<
							 " Texture Size "<< m_fboTexWidth << " "<<m_fboTexHeight<<"\n";

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void OculusInterface::createOVRTextureBuffers()
{
	// left eye
	m_fbTextureIDOVR[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	m_fbTextureIDOVR[0].OGL.Header.TextureSize.w = m_fboTexWidth;
	m_fbTextureIDOVR[0].OGL.Header.TextureSize.h = m_fboTexHeight;
	m_fbTextureIDOVR[0].OGL.Header.RenderViewport.Pos.x = 0;
	m_fbTextureIDOVR[0].OGL.Header.RenderViewport.Pos.y = m_fboTexHeight-m_fbHeight;
	m_fbTextureIDOVR[0].OGL.Header.RenderViewport.Size.w = m_fbWidth / 2.0;
	m_fbTextureIDOVR[0].OGL.Header.RenderViewport.Size.h = m_fbHeight;
	m_fbTextureIDOVR[0].OGL.TexId = m_fboTex;	/* both eyes will use the same texture id */
	// Right Eye
	m_fbTextureIDOVR[1].OGL.Header.API = ovrRenderAPI_OpenGL;
	m_fbTextureIDOVR[1].OGL.Header.TextureSize.w = m_fboTexWidth;
	m_fbTextureIDOVR[1].OGL.Header.TextureSize.h = m_fboTexHeight;
	m_fbTextureIDOVR[1].OGL.Header.RenderViewport.Pos.x = m_fbWidth / 2.0;
	m_fbTextureIDOVR[1].OGL.Header.RenderViewport.Pos.y = m_fboTexHeight-m_fbHeight;
	m_fbTextureIDOVR[1].OGL.Header.RenderViewport.Size.w = m_fbWidth / 2.0;
	m_fbTextureIDOVR[1].OGL.Header.RenderViewport.Size.h = m_fbHeight;
	m_fbTextureIDOVR[1].OGL.TexId = m_fboTex;	/* both eyes will use the same texture id */



}

void OculusInterface::createOVRGLConfig()
{
	std::cout<<"Create GL Config\n";
	memset(&m_glcfg, 0, sizeof m_glcfg);
	m_glcfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	m_glcfg.OGL.Header.RTSize = m_hmd->Resolution;
	m_glcfg.OGL.Header.RTSize.w*=m_devicePixelAspect;
	m_glcfg.OGL.Header.RTSize.h*=m_devicePixelAspect;

	m_glcfg.OGL.Header.Multisample = 1;

}

void OculusInterface::beginFrame()
{
	//std::cout<<"Begin Frame\n";
	ovrHmd_BeginFrame(m_hmd, 0);
	/* start drawing onto our texture render target */
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




}


void OculusInterface::endFrame()
{
	//std::cout<<"End Frame\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_windowWidth, m_windowHeight);

	ovrHmd_EndFrame(m_hmd, m_pose, &m_fbTextureIDOVR[0].Texture);
	//assert(glGetError() == GL_NO_ERROR);

}


void OculusInterface::setLeftEye()
{
	ovrEyeType eye = m_hmd->EyeRenderOrder[0];
	glViewport(0, 0, (m_fbWidth / 2), m_fbHeight);
	m_pose[0] = ovrHmd_GetEyePose(m_hmd, eye);

}

void OculusInterface::setRightEye()
{
	ovrEyeType eye = m_hmd->EyeRenderOrder[1];
	glViewport( (m_fbWidth / 2),0, (m_fbWidth / 2), m_fbHeight);
	m_pose[1] = ovrHmd_GetEyePose(m_hmd, eye);

}

void OculusInterface::disableWarningMessage()
{
	// Health and Safety Warning display state.
	ovrHSWDisplayState hswDisplayState;
	ovrHmd_GetHSWDisplayState(m_hmd, &hswDisplayState);
	if (hswDisplayState.Displayed)
	{
	ovrHmd_DismissHSWDisplay(m_hmd);
	}
}
ngl::Mat4 OculusInterface::getPerspectiveMatrix(int _eye)
{
	ovrMatrix4f proj;
	proj = ovrMatrix4f_Projection(m_hmd->DefaultEyeFov[_eye], 0.5, 2500.0, 1);
	ngl::Mat4 mat;
	memcpy(&mat.m_openGL[0],&proj.M,sizeof(float)*16 );
	mat.transpose();
	return mat;
}

ngl::Vec3 OculusInterface::getLeftEyeOffset() const
{
return ngl::Vec3(m_eyeRdesc[0].ViewAdjust.x, m_eyeRdesc[0].ViewAdjust.y, m_eyeRdesc[0].ViewAdjust.z);
}
ngl::Vec3 OculusInterface::getRightEyeOffset() const
{
	return ngl::Vec3(m_eyeRdesc[1].ViewAdjust.x, m_eyeRdesc[1].ViewAdjust.y, m_eyeRdesc[1].ViewAdjust.z);

}

ngl::Mat4 OculusInterface::getViewMatrix(int _eye)
{
	m_pose[_eye] = ovrHmd_GetEyePose(m_hmd,(ovrEyeType) _eye);
	ngl::Mat4 pos;
	pos.translate(m_eyeRdesc[_eye].ViewAdjust.x, m_eyeRdesc[_eye].ViewAdjust.y, m_eyeRdesc[_eye].ViewAdjust.z);
	ngl::Mat4 rotation ;
	ngl::Quaternion orientation(m_pose[_eye].Orientation.w,m_pose[_eye].Orientation.x,m_pose[_eye].Orientation.y,m_pose[_eye].Orientation.z);
	//	quat_to_matrix(&m_pose[_eye].Orientation.x, &rotation.m_m[0]);
	rotation=orientation.toMat4();
	rotation.transpose();
	ngl::Mat4  eyePos;
	eyePos.translate(-m_pose[_eye].Position.x, -m_pose[_eye].Position.y, -m_pose[_eye].Position.z);
	ngl::Mat4 eyeLevel;
	eyeLevel.translate(0, -ovrHmd_GetFloat(m_hmd, OVR_KEY_EYE_HEIGHT, 1.65), 0);
	// could optimize this
	return pos*rotation*eyePos*eyeLevel;

}
