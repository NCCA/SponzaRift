#ifndef OCULUSINTERFACE_H__
#define OCULUSINTERFACE_H__
#include <OVR.h>

class OculusInterface
{
  public :
    /// @brief to return the instance of the class
    /// @returns the constructed class
    static OculusInterface* instance();
    inline void debugOn(){m_debug=true;}
    inline void debugOff(){m_debug=false;}
    inline void turnOffWarning(){m_warningOff=true;}
    void oculusDebug() const;
    void releaseOculus();
    void oculusDisplayWarning();
    void oculusPoseState();

  private :
    // singleton
    OculusInterface();
    ~OculusInterface(){;}
    OculusInterface(const OculusInterface &_t){;}
    /// @brief the instance of the class
    static OculusInterface* m_pinstance;
    /// @brief debug flag
    bool m_debug;
    // Oculus head mounted display pointer
    ovrHmd m_hmd;
    // turn off the warning
    bool m_warningOff;



};



#endif
