#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Transformation.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include "OculusInterface.h"
#include <ngl/NGLStream.h>

//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=2.0f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=20.0f;

NGLScene::NGLScene(QWindow *_parent) : OpenGLWindow(_parent)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Sponza Demo");
  m_whichMap=0;
  m_single=false;

}


NGLScene::~NGLScene()
{
  ngl::NGLInit *Init = ngl::NGLInit::instance();
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
 // delete m_mtl;
 // delete m_model;
  Init->NGLQuit();
  //OculusInterface::instance()->releaseOculus();
  m_ovr->releaseOculus();
}

void NGLScene::resizeEvent(QResizeEvent *_event )
{
  if(isExposed())
  {
    setWidth(m_ovr->getWidth());
    setHeight(m_ovr->getHeight());

    // set the viewport for openGL
  //glViewport(0,0,w,h);
  // now set the camera size values as the screen size has changed
  //m_cam->setShape(45,(float)w/h,0.05,350);
  renderLater();
  }
}


void NGLScene::initialize()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();
  //OculusInterface::instance();
  m_ovr = OculusInterface::instance();
  m_ovr->initOculus(devicePixelRatio());
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);

   // now to load the shader and set the values
  // grab an instance of shader manager
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // load a frag and vert shaders

  shader->createShaderProgram("TextureShader");

  shader->attachShader("TextureVertex",ngl::VERTEX);
  shader->attachShader("TextureFragment",ngl::FRAGMENT);
  shader->loadShaderSource("TextureVertex","shaders/TextureVert.glsl");
  shader->loadShaderSource("TextureFragment","shaders/TextureFrag.glsl");
  //shader->loadShaderSource("TextureVertex","shaders/NormalMapVert.glsl");
  //shader->loadShaderSource("TextureFragment","shaders/NormalMapFrag.glsl");

  shader->compileShader("TextureVertex");
  shader->compileShader("TextureFragment");
  shader->attachShaderToProgram("TextureShader","TextureVertex");
  shader->attachShaderToProgram("TextureShader","TextureFragment");
  // bind our attributes for the vertex shader

  // link the shader no attributes are bound
  shader->linkProgramObject("TextureShader");
  (*shader)["TextureShader"]->use();
//  shader->setShaderParam1i("tex",0);
//  shader->setShaderParam1i("spec",1);
//  shader->setShaderParam1i("normalMap",2);

  shader->setShaderParam1i("ambientMap",1);
  shader->setShaderParam1i("diffuseMap",0);
  shader->setShaderParam1i("normalMap",2);
  ngl::Mat4 iv;
  iv.transpose();

  /// now setup a basic 3 point lighting system
  m_key= new ngl::Light(ngl::Vec3(3,2,2),ngl::Colour(1,1,1,1),ngl::POINTLIGHT);
  m_key->setTransform(iv);
  m_key->enable();
  m_key->loadToShader("light[0]");
  m_fill = new ngl::Light(ngl::Vec3(-3,1.5,2),ngl::Colour(1,1,1,1),ngl::POINTLIGHT);
  m_fill->setTransform(iv);
  m_fill->enable();
  m_fill->loadToShader("light[1]");

  m_back= new ngl::Light(ngl::Vec3(0,1,-2),ngl::Colour(1,1,1,1),ngl::POINTLIGHT);
  m_back->setTransform(iv);
  m_back->enable();
  m_back->loadToShader("light[2]");

  shader->setUniform("light.position",0,40,0);
  shader->setShaderParam3f("light.La",0.1,0.1,0.1);
  shader->setShaderParam3f("light.Ld",1.0,1.0,1.0);
  shader->setShaderParam3f("light.Ls",0.9,0.9,0.9);
  //shader->setShaderParam3f("Falloff",1.0f, 1.0f, 1.0f);
  //shader->setShaderParam2f("Resolution",float(width()),float(height()));


  glEnable(GL_DEPTH_TEST);

  m_mtl = new Mtl;
  bool loaded=m_mtl->loadBinary("models/sponzaMtl.bin");
  //bool loaded=m_mtl->load("models/sponza.mtl");
  //m_mtl->saveBinary("models/sponzaMtl.bin");
  if(loaded == false)
  {
    std::cerr<<"error loading mtl file ";
    exit(EXIT_FAILURE);
  }


  m_model = new GroupedObj();//"models/sponza.obj");
  //m_model->saveBinary("models/SponzaMesh.bin");
  loaded=m_model->loadBinary("models/SponzaMesh.bin");
  if(loaded == false)
  {
    std::cerr<<"error loading obj file ";
    exit(EXIT_FAILURE);
  }

  // as re-size is not explicitly called we need to do this.
 // glViewport(0,0,width(),height());
  startTimer(0);
 // m_ovr->disableWarningMessage();
}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV= m_transform.getMatrix()*m_mouseGlobalTX*m_view;
  MVP=MV*m_projection ;
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setUniform("MVP",MVP);
  shader->setUniform("MV",MV);
  shader->setUniform("normalMatrix",normalMatrix);



 }

void NGLScene::render()
{

  // Rotation based on the mouse position for our global transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(m_single)
  {
    glViewport(0,0,width()*devicePixelRatio() ,height()*devicePixelRatio());
    m_projection=ngl::perspective(55,float(width()/height()),0.1,500);
    m_view = ngl::lookAt(ngl::Vec3(0,40,-240),ngl::Vec3(0,0,0),ngl::Vec3(0,1,0));
    drawScene(0);
  }

  else
  {
    m_ovr->beginFrame();

    for(int eye=0; eye<2; ++eye)
    {
      if(eye==0)
      {
        m_ovr->setLeftEye();

      }
      else
      {
        m_ovr->setRightEye();

      }
      m_projection=m_ovr->getPerspectiveMatrix(eye);
      m_view=m_ovr->getViewMatrix(eye);

      drawScene(eye);
    }// for each eye
    m_ovr->endFrame();

  }

}


void NGLScene::drawScene(int _eye)
{
//  m_projection=m_ovr->getPerspectiveMatrix(_eye);
//  m_view=m_ovr->getViewMatrix(_eye);
  loadMatricesToShader();
  unsigned int end=m_model->numMeshes();
  std::string matName;
  for(unsigned int i=0; i<end; ++i)
  {
    //m_mtl->use(m_model->getMaterial(i));
    mtlItem *currMaterial=m_mtl->find(m_model->getMaterial(i));
    if(currMaterial == 0) continue;
    // see if we need to switch the material or not this saves on OpenGL calls and
    // should speed things up
    if(matName !=m_model->getMaterial(i))
    {
      matName=m_model->getMaterial(i);
//      switch(m_whichMap)
//      {
//        case 0 : glBindTexture (GL_TEXTURE_2D,currMaterial->map_KaId); break;
//        case 1 : glBindTexture (GL_TEXTURE_2D,currMaterial->map_KdId); break;
//        case 2 : glBindTexture (GL_TEXTURE_2D,currMaterial->map_bumpId); break;
//        case 3 : glBindTexture (GL_TEXTURE_2D,currMaterial->bumpId); break;
//        case 4 : glBindTexture (GL_TEXTURE_2D,currMaterial->map_dId); break;
//      }
      ngl::ShaderLib *shader = ngl::ShaderLib::instance();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_2D,currMaterial->map_KaId);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture (GL_TEXTURE_2D,currMaterial->map_KdId);
      if(currMaterial->bumpId !=0)
      {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture (GL_TEXTURE_2D,currMaterial->bumpId);
      }


      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
      shader->setShaderParam3f("ka",currMaterial->Ka.m_x,currMaterial->Ka.m_y,currMaterial->Ka.m_z);
      shader->setShaderParam3f("kd",currMaterial->Kd.m_x,currMaterial->Kd.m_y,currMaterial->Kd.m_z);
      shader->setShaderParam1f("transp",currMaterial->d);
    }
    m_model->draw(i);

  }

  //ngl::VAOPrimitives::instance()->draw("bunny");
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    renderLater();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    renderLater();

   }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
   if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

	// check the diff of the wheel position (0 means no change)
	if(_event->delta() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->delta() <0 )
	{
		m_modelPos.m_z-=ZOOM;
	}
	renderLater();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  default : break;
  case Qt::Key_1 : m_whichMap=0; break;
  case Qt::Key_2 : m_whichMap=1; break;
  case Qt::Key_3 : m_whichMap=2; break;
  case Qt::Key_4 : m_whichMap=3; break;
  case Qt::Key_5 : m_whichMap=4; break;
  case Qt::Key_Space : m_ovr->disableWarningMessage(); break;
  case Qt::Key_A : m_single^=true; break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    renderLater();
}





void NGLScene::timerEvent(QTimerEvent *)
{
 // OculusInterface *interface = OculusInterface::instance();
 // m_ovr->oculusPoseState();
  renderLater();
}

