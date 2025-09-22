#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgText/Text>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/GUIEventHandler>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include "OsgImGuiHandler.hpp" // Your wrapper to integrate ImGui with OSG

// --- Helper to create one axis ---
osg::ref_ptr<osg::MatrixTransform> createAxis(
    const osg::Vec3 &axisDir,
    const osg::Vec4 &color,
    const std::string &label,
    float length = 5.0f,
    float radius = 0.1f,
    float coneRadius = 0.2f,
    float coneHeight = 0.5f)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::ref_ptr<osg::Cylinder> bar =
        new osg::Cylinder(osg::Vec3(0, 0, length * 0.5f), radius, length);
    osg::ref_ptr<osg::ShapeDrawable> barDrawable = new osg::ShapeDrawable(bar);
    barDrawable->setColor(color);
    geode->addDrawable(barDrawable);

    osg::ref_ptr<osg::Cone> arrow =
        new osg::Cone(osg::Vec3(0, 0, length), coneRadius, coneHeight);
    osg::ref_ptr<osg::ShapeDrawable> arrowDrawable = new osg::ShapeDrawable(arrow);
    arrowDrawable->setColor(color);
    geode->addDrawable(arrowDrawable);

    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setFont("arial.ttf");
    text->setCharacterSize(0.7f);
    text->setAxisAlignment(osgText::Text::SCREEN);
    text->setPosition(osg::Vec3(0, 0, length + coneHeight + 0.2f));
    text->setText(label);
    text->setColor(color);
    geode->addDrawable(text);

    osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
    osg::Quat rot;
    rot.makeRotate(osg::Vec3(0, 0, 1), axisDir);
    mt->setMatrix(osg::Matrix::rotate(rot));
    mt->addChild(geode);

    return mt;
}

// --- Helper: create full XYZ triad ---
// For body axes, label stays BodyX/Y/Z, but direction can match NED
osg::ref_ptr<osg::Group> createAxes(const std::string &prefix = "",
                                    bool ned = false,
                                    bool bodyFrame = false)
{
    osg::ref_ptr<osg::Group> axes = new osg::Group;

    osg::ref_ptr<osg::Geode> originGeode = new osg::Geode;
    osg::ref_ptr<osg::Sphere> sphere = new osg::Sphere(osg::Vec3(0, 0, 0), 0.3f);
    osg::ref_ptr<osg::ShapeDrawable> sphereDrawable = new osg::ShapeDrawable(sphere);
    sphereDrawable->setColor(osg::Vec4(1, 1, 1, 1));
    originGeode->addDrawable(sphereDrawable);
    axes->addChild(originGeode);

    if (!ned)
    {
        axes->addChild(createAxis(osg::Vec3(1, 0, 0), osg::Vec4(1, 0, 0, 1), prefix + "X"));
        axes->addChild(createAxis(osg::Vec3(0, 1, 0), osg::Vec4(0, 1, 0, 1), prefix + "Y"));
        axes->addChild(createAxis(osg::Vec3(0, 0, 1), osg::Vec4(0, 0, 1, 1), prefix + "Z"));
    }
    else
    {
        if (bodyFrame)
        {
            // Body axes: direction matches NED, labels remain BodyX/Y/Z
            axes->addChild(createAxis(osg::Vec3(-1, 0, 0), osg::Vec4(1, 0, 0, 1), prefix + "X")); // nose -> North
            axes->addChild(createAxis(osg::Vec3(0, 1, 0), osg::Vec4(0, 1, 0, 1), prefix + "Y"));  // right wing -> East
            axes->addChild(createAxis(osg::Vec3(0, 0, -1), osg::Vec4(0, 0, 1, 1), prefix + "Z")); // bottom -> Down
        }
        else
        {
            // Reference axes: show N/E/D
            axes->addChild(createAxis(osg::Vec3(-1, 0, 0), osg::Vec4(1, 0, 0, 1), prefix + "N"));
            axes->addChild(createAxis(osg::Vec3(0, 1, 0), osg::Vec4(0, 1, 0, 1), prefix + "E"));
            axes->addChild(createAxis(osg::Vec3(0, 0, -1), osg::Vec4(0, 0, 1, 1), prefix + "D"));
        }
    }

    return axes;
}

// --- Event handler for keyboard control ---
class FighterControlHandler : public osgGA::GUIEventHandler
{
public:
    FighterControlHandler(osg::ref_ptr<osg::MatrixTransform> fighterModel,
                          osg::ref_ptr<osg::MatrixTransform> fighterAxes)
        : _fighter(fighterModel), _axes(fighterAxes), _pitch(0), _yaw(0), _roll(0)
    {
    }

    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &) override
    {
        if (!_fighter)
            return false;

        switch (ea.getEventType())
        {
        case osgGA::GUIEventAdapter::KEYDOWN:
            switch (ea.getKey())
            {
            case 'q':
                _pitch += 0.05;
                break;
            case 'a':
                _pitch -= 0.05;
                break;
            case 'w':
                _yaw += 0.05;
                break;
            case 's':
                _yaw -= 0.05;
                break;
            case 'e':
                _roll += 0.05;
                break;
            case 'd':
                _roll -= 0.05;
                break;
            case 'r':
                resetRotation();
                break;
            default:
                return false;
            }
            updateRotation();
            return true;
        default:
            return false;
        }
    }

private:
    void updateRotation()
    {
        osg::Quat qPitch(_pitch, osg::Vec3(1, 0, 0));
        osg::Quat qYaw(_yaw, osg::Vec3(0, 0, 1));
        osg::Quat qRoll(_roll, osg::Vec3(0, 1, 0));

        osg::Quat finalRot = qYaw * qPitch * qRoll;

        _fighter->setMatrix(osg::Matrix::rotate(finalRot));
        if (_axes)
            _axes->setMatrix(osg::Matrix::rotate(finalRot));
    }

    void resetRotation()
    {
        _pitch = _yaw = _roll = 0.0;
        updateRotation();
    }

    osg::ref_ptr<osg::MatrixTransform> _fighter;
    osg::ref_ptr<osg::MatrixTransform> _axes;
    double _pitch, _yaw, _roll;
};

class ImGuiPlaneHandler : public OsgImGuiHandler
{
public:
    ImGuiPlaneHandler(osg::ref_ptr<osg::MatrixTransform> modelTransform,
                      osg::ref_ptr<osg::MatrixTransform> axesTransform)
        : planeTransform_(modelTransform), axesTransform_(axesTransform)
    {
        roll = pitch = yaw = 0.0f;
    }

    void drawUi() override
    {
        ImGui::Begin("Plane Rotation");

        // Roll
        ImGui::Text("Roll");
        ImGui::SliderFloat("##RollSlider", &roll, -180.0f, 180.0f);
        ImGui::SameLine();
        ImGui::InputFloat("##RollInput", &roll, 1.0f, 10.0f, "%.1f");

        // Pitch
        ImGui::Text("Pitch");
        ImGui::SliderFloat("##PitchSlider", &pitch, -180.0f, 180.0f);
        ImGui::SameLine();
        ImGui::InputFloat("##PitchInput", &pitch, 1.0f, 10.0f, "%.1f");

        // Yaw
        ImGui::Text("Yaw");
        ImGui::SliderFloat("##YawSlider", &yaw, -180.0f, 180.0f);
        ImGui::SameLine();
        ImGui::InputFloat("##YawInput", &yaw, 1.0f, 10.0f, "%.1f");

        // Reset button
        if (ImGui::Button("Reset"))
        {
            roll = pitch = yaw = 0.0f;
        }

        ImGui::End();

        // ------------------------
        // Apply rotations
        // ------------------------

        // 1) Plane model (viewer2) - rotate around **local axes**
        osg::Quat qRoll_model(osg::DegreesToRadians(roll), osg::Vec3(1,0,0));    // BX
        osg::Quat qPitch_model(osg::DegreesToRadians(pitch), osg::Vec3(0,1,0));  // BY
        osg::Quat qYaw_model(osg::DegreesToRadians(yaw), osg::Vec3(0,0,1));      // BZ

        osg::Quat modelRot = qYaw_model * qPitch_model * qRoll_model;
        planeTransform_->setMatrix(osg::Matrix::rotate(modelRot));

        // 2) Body axes (viewer1) - rotate relative to NED/world axes
        if (axesTransform_)
        {
            // Keep axes aligned to NED but show plane orientation
            osg::Quat qRoll_axes(osg::DegreesToRadians(roll), osg::Vec3(1,0,0));   // rotate around world X
            osg::Quat qPitch_axes(osg::DegreesToRadians(pitch), osg::Vec3(0,1,0)); // rotate around world Y
            osg::Quat qYaw_axes(osg::DegreesToRadians(yaw), osg::Vec3(0,0,1));     // rotate around world Z

            osg::Quat axesRot = qYaw_axes * qPitch_axes * qRoll_axes;
            axesTransform_->setMatrix(osg::Matrix::rotate(axesRot));
        }
    }

private:
    osg::ref_ptr<osg::MatrixTransform> planeTransform_;
    osg::ref_ptr<osg::MatrixTransform> axesTransform_;
    float roll, pitch, yaw;
};

class ImGuiInitOperation : public osg::Operation
{
public:
    ImGuiInitOperation()
        : osg::Operation("ImGuiInitOperation", false) {}

    virtual void operator()(osg::Object * /*object*/) override
    {
        if (!ImGui_ImplOpenGL3_Init())
        {
            std::cout << "ImGui_ImplOpenGL3_Init() failed\n";
        }
    }
};

int main()
{
    // ---------------------------
    // Left window (axes display)
    // ---------------------------
    osg::ref_ptr<osg::Group> root1 = new osg::Group;
    root1->addChild(createAxes("", true)); // Reference axes: NED

    osg::ref_ptr<osg::MatrixTransform> fighterAxesTransform = new osg::MatrixTransform;
    fighterAxesTransform->addChild(createAxes("B", true, true)); // Body axes: NED but labels BodyX/Y/Z
    root1->addChild(fighterAxesTransform);

    osgViewer::Viewer viewer1;
    viewer1.setSceneData(root1.get());
    viewer1.setUpViewInWindow(50, 50, 600, 600);
    viewer1.getCamera()->setViewMatrixAsLookAt(
        osg::Vec3(20, 20, 20),
        osg::Vec3(0, 0, 0),
        osg::Vec3(0, 0, 1));

    // ---------------------------
    // Right window (Cessna model)
    // ---------------------------
    osg::ref_ptr<osg::Group> root2 = new osg::Group;
    std::string dataPath = "/home/murate/Documents/SwTrn/OsgTrn/OpenSceneGraph-Data/";
    osg::ref_ptr<osg::Node> cessnaModel = osgDB::readRefNodeFile(dataPath + "F-14-low-poly-osg.ac");
    if (!cessnaModel)
    {
        osg::notify(osg::FATAL) << "Could not load cessna.osgt" << std::endl;
        return 1;
    }

    osg::ref_ptr<osg::MatrixTransform> fighterModelTransform = new osg::MatrixTransform;
    fighterModelTransform->addChild(cessnaModel);
    root2->addChild(fighterModelTransform);

    // Right window (Cessna model)
    osgViewer::Viewer viewer2;
    viewer2.setSceneData(root2.get());
    viewer2.setUpViewInWindow(700, 50, 600, 600);
    viewer2.setCameraManipulator(new osgGA::TrackballManipulator);
    viewer2.home();

    // Initialize ImGui (once, before main loop)
    viewer2.setRealizeOperation(new ImGuiInitOperation);

    // Add ImGui handler
    viewer2.addEventHandler(new ImGuiPlaneHandler(fighterModelTransform, fighterAxesTransform));

    // ---------------------------
    // Main loop
    // ---------------------------
    while (!viewer1.done() && !viewer2.done())
    {
        viewer1.frame();
        viewer2.frame();
    }

    return 0;
}