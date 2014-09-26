#include "OculusInterface.h"
#include <iostream>
OculusInterface* OculusInterface::m_pinstance = 0;// initialize pointer

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
  oculusDebug();
  // Start the sensor which provides the Rift’s pose and motion.
  ovrHmd_ConfigureTracking(m_hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
  // Query the HMD for the current tracking state.
oculusDisplayWarning();

}


void OculusInterface::oculusDebug() const
{
  if(m_debug)
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



