#include <irrlicht.h>
#include <stdio.h>
#include <stdlib.h>

using namespace irr;

using namespace core;
using namespace io;
using namespace gui;

#pragma comment(lib, "Irrlicht.lib")

IrrlichtDevice *Device = 0;
s32 cnt = 0;
core::stringc StartUpModelFile;
core::stringw MessageText;
core::stringw Caption;
scene::IAnimatedMeshSceneNode* Model = 0;
scene::ISceneNode* SkyBox = 0;

void showAboutText()
{
	// create modal message box with the text loaded from the
	// xml file.
	Device->getGUIEnvironment()->addMessageBox(
		Caption.c_str(), MessageText.c_str());
}


void loadModel(const c8* filename)
{
	// load a model into the engine

	if (Model)
		Model->remove();

	scene::IAnimatedMesh* m = Device->getSceneManager()->getMesh(filename);

	if (!m) 
	{
		// model could not be loaded

		if (StartUpModelFile != filename)
			Device->getGUIEnvironment()->addMessageBox(
			Caption.c_str(), L"The model could not be loaded. " \
			L"Maybe it is not a supported file format.");
		return;
	}

	// set default material properties

	Model = Device->getSceneManager()->addAnimatedMeshSceneNode(m);
	Model->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	Model->setMaterialFlag(video::EMF_LIGHTING, false);
	Model->setDebugDataVisible(true);
}


class MyEventReceiver : public IEventReceiver
{
public:
	virtual bool OnEvent(SEvent event)
	{
		if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
			IGUIEnvironment* env = Device->getGUIEnvironment();

			switch(event.GUIEvent.EventType)
			{
			case EGET_MENU_ITEM_SELECTED:
				{
					// a menu item was clicked

					IGUIContextMenu* menu = (IGUIContextMenu*)event.GUIEvent.Caller;
					s32 id = menu->getItemCommandId(menu->getSelectedItem());
					
					switch(id)
					{
					case 100: // File -> Open Model
						env->addFileOpenDialog(L"Please select a model file to open");
						break;
					case 200: // File -> Quit
						Device->closeDevice();
						break;
					case 300: // View -> Skybox
						SkyBox->setVisible(!SkyBox->isVisible());
						break;
					case 400: // View -> Debug Information
						if (Model)
							Model->setDebugDataVisible(!Model->isDebugDataVisible());
						break;
					case 500: // Help->About
						showAboutText();
						break;
					case 610: // View -> Material -> Solid
						if (Model)
							Model->setMaterialType(video::EMT_SOLID);
						break;
					case 620: // View -> Material -> Transparent
						if (Model)
							Model->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
						break;
					case 630: // View -> Material -> Reflection
						if (Model)
							Model->setMaterialType(video::EMT_SPHERE_MAP);
						break;
					}
                    break;
				}

			case EGET_FILE_SELECTED:
				{
					IGUIFileOpenDialog* dialog = (IGUIFileOpenDialog*)event.GUIEvent.Caller;
					loadModel(core::stringc(dialog->getFilename()).c_str());
				}

			case EGET_BUTTON_CLICKED:

				if (id == 1101)
				{
					// set scale
					gui::IGUIElement* root = env->getRootGUIElement();
					core::vector3df scale;
					core::stringc s;

					s = root->getElementFromId(901, true)->getText();
					scale.X = (f32)atof(s.c_str());
					s = root->getElementFromId(902, true)->getText();
					scale.Y = (f32)atof(s.c_str());
					s = root->getElementFromId(903, true)->getText();
					scale.Z = (f32)atof(s.c_str());

					if (Model)
						Model->setScale(scale);
				}

				break;
			}
		}

		return false;
	}
};



int main()
{
	// start up the engine 

	MyEventReceiver receiver;

	Device = createDevice(video::EDT_DIRECTX8,
		core::dimension2d<s32>(640, 480), 16, false, false, &receiver);

	Device->setWindowCaption(L"Irrlicht Engine - Loading...");

	video::IVideoDriver* driver = Device->getVideoDriver();
	IGUIEnvironment* env = Device->getGUIEnvironment();
	scene::ISceneManager* smgr = Device->getSceneManager();

	// read configuration from xml file

	io::IXMLReader* xml = Device->getFileSystem()->createXMLReader("config.xml");


	while(xml && xml->read())
	{
		switch(xml->getNodeType())
		{
		case io::EXN_TEXT:
			// in this xml file, the only text which occurs is the messageText
			MessageText = xml->getNodeData();
			break;
		case io::EXN_ELEMENT:
			{
				if (core::stringw("startUpModel") == xml->getNodeName())
					StartUpModelFile = xml->getAttributeValue(L"file");
				else
				if (core::stringw("messageText") == xml->getNodeName())
					Caption = xml->getAttributeValue(L"caption");
			}
			break;
		}
	}

	if (xml)
		xml->drop(); // don't forget to delete the xml reader 


	// set a nicer font

	IGUISkin* skin = env->getSkin();
	IGUIFont* font = env->getFont("Models/fonthaettenschweiler.bmp");
	if (font)
		skin->setFont(font);

	// create menu

	gui::IGUIContextMenu* menu = env->addMenu();
	menu->addItem(L"File", -1, true, true);
	menu->addItem(L"View", -1, true, true);
	menu->addItem(L"Help", -1, true, true);

	gui::IGUIContextMenu* submenu;
	submenu = menu->getSubMenu(0);
	submenu->addItem(L"Open Model File...", 100);
	submenu->addSeparator();
	submenu->addItem(L"Quit", 200);

	submenu = menu->getSubMenu(1);
	submenu->addItem(L"toggle sky box visibility", 300);
	submenu->addItem(L"toggle model debug information", 400);
	submenu->addItem(L"model material", -1, true, true );

	submenu = submenu->getSubMenu(2);
	submenu->addItem(L"Solid", 610);
	submenu->addItem(L"Transparent", 620);
	submenu->addItem(L"Reflection", 630);

	submenu = menu->getSubMenu(2);
	submenu->addItem(L"About", 500);

	// add a tabcontrol

	IGUITabControl* tab = env->addTabControl(
		core::rect<s32>(450,18,640,480), 0, true, true);

	IGUITab* t1 = tab->addTab(L"Scale");
	IGUITab* t2 = tab->addTab(L"Empty Tab");

	env->addEditBox(L"1.0", core::rect<s32>(40,50,130,70), true, t1, 901);
	env->addEditBox(L"1.0", core::rect<s32>(40,80,130,100), true, t1, 902);
	env->addEditBox(L"1.0", core::rect<s32>(40,110,130,130), true, t1, 903);

	env->addButton(rect<s32>(10,150,100,190), t1, 1101, L"set");


	// add skybox 

	SkyBox = smgr->addSkyBoxSceneNode(
		driver->getTexture("Models/irrlicht2_up.jpg"),
		driver->getTexture("Models/irrlicht2_dn.jpg"),
		driver->getTexture("Models/irrlicht2_lf.jpg"),
		driver->getTexture("Models/irrlicht2_rt.jpg"),
		driver->getTexture("Models/irrlicht2_ft.jpg"),
		driver->getTexture("Models/irrlicht2_bk.jpg"));

	// add a camera scene node 

	smgr->addCameraSceneNodeMaya();
	
	// add the irrlicht engine logo

	IGUIImage* img = env->addImage(rect<s32>(640-98-10,480-41-10,640-20,480-20));
	img->setImage(driver->getTexture("Models/irrlichtlogoaligned.jpg"));

	// create fps text 

	IGUIStaticText* fpstext = env->addStaticText(L"", rect<s32>(10,455,60,470), true);
	fpstext->setOverrideColor(video::SColor(255,255,255,255));

	// set window caption

	Caption += " - [";
	Caption += driver->getName();
	Caption += "]";
	Device->setWindowCaption(Caption.c_str());

	// show about message box and load default model
	showAboutText();
	loadModel(StartUpModelFile.c_str());

	// draw everything

	while(Device->run() && driver)
		if (Device->isWindowActive())
		{
			driver->beginScene(true, true, video::SColor(150,50,50,50));

			smgr->drawAll();
			env->drawAll();
		
			driver->endScene();

			wchar_t tmp[255];
			swprintf(tmp, 255, L"FPS: %d", driver->getFPS());
			fpstext->setText(tmp);
		}

	Device->drop();
	return 0;
}
