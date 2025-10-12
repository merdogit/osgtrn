// Model konum-yonelim kalibrasyonu

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/NodeTrackerManipulator>
#include <osgGA/GUIEventHandler>
#include <osg/AnimationPath>
#include <iostream>
#include <cmath>

class ModelAdjustHandler : public osgGA::GUIEventHandler
{
public:
    ModelAdjustHandler(osg::MatrixTransform *model) : _model(model)
    {
    }

    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &) override
    {
        if (!_model)
            return false;

        if (ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN)
            return false;

        osg::Matrix m = _model->getMatrix();
        osg::Vec3 trans = m.getTrans();
        osg::Quat rot = m.getRotate();
        float step = 0.1f;
        float angleStep = osg::DegreesToRadians(1.0f);

        switch (ea.getKey())
        {
        // Translation
        case osgGA::GUIEventAdapter::KEY_Left:
            trans.x() -= step;
            break;
        case osgGA::GUIEventAdapter::KEY_Right:
            trans.x() += step;
            break;
        case osgGA::GUIEventAdapter::KEY_Up:
            trans.y() += step;
            break;
        case osgGA::GUIEventAdapter::KEY_Down:
            trans.y() -= step;
            break;
        case osgGA::GUIEventAdapter::KEY_Page_Up:
            trans.z() += step;
            break;
        case osgGA::GUIEventAdapter::KEY_Page_Down:
            trans.z() -= step;
            break;

        // Rotation
        case 'q':
            rot = osg::Quat(angleStep, osg::Vec3(0, 0, 1)) * rot;
            break; // yaw left
        case 'e':
            rot = osg::Quat(-angleStep, osg::Vec3(0, 0, 1)) * rot;
            break; // yaw right
        case 'w':
            rot = osg::Quat(angleStep, osg::Vec3(1, 0, 0)) * rot;
            break; // pitch up
        case 's':
            rot = osg::Quat(-angleStep, osg::Vec3(1, 0, 0)) * rot;
            break; // pitch down
        case 'a':
            rot = osg::Quat(angleStep, osg::Vec3(0, 1, 0)) * rot;
            break; // roll left
        case 'd':
            rot = osg::Quat(-angleStep, osg::Vec3(0, 1, 0)) * rot;
            break; // roll right
        case 'r':
            _model->setMatrix(_model->getInverseMatrix() *
                              _model->getMatrix());
            trans = _model->getMatrix().getTrans();
            rot = _model->getMatrix().getRotate();
            break;
        default:
            return false;
        }

        _model->setMatrix(osg::Matrix::rotate(rot) * osg::Matrix::translate(trans));
        std::cout << "Marker position: "
                  << trans.x() << ", " << trans.y() << ", " << trans.z()
                  << " | rotation: " << rot.x() << ", " << rot.y() << ", "
                  << rot.z() << ", " << rot.w() << std::endl;
        return true;
    }

private:
    osg::observer_ptr<osg::MatrixTransform> _model;
};

int main()
{
    osg::ref_ptr<osg::Group> root = new osg::Group();

    std::string dataPath = "/home/murate/Documents/SwTrn/OsgTrn/OpenSceneGraph-Data/";
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(dataPath + "AIM-9L.ac");
    if (!model)
    {
        std::cerr << "Cannot load model\n";
        return 1;
    }

    osg::ref_ptr<osg::MatrixTransform> modelXForm = new osg::MatrixTransform;
    modelXForm->addChild(model);

    osg::ref_ptr<osg::Node> refAxes = osgDB::readNodeFile(dataPath + "axes.osgt");
    osg::ref_ptr<osg::MatrixTransform> refAxesXForm = new osg::MatrixTransform;
    refAxesXForm->addChild(refAxes);
    refAxesXForm->setMatrix(osg::Matrix::scale(2.0f, 2.0f, 2.0f));

    root->addChild(modelXForm);
    root->addChild(refAxesXForm);

    osg::ref_ptr<ModelAdjustHandler> modelHandler = new ModelAdjustHandler(modelXForm.get());

    osgViewer::Viewer viewer;
    viewer.addEventHandler(modelHandler.get());
    viewer.setUpViewInWindow(700, 50, 800, 600);
    viewer.setSceneData(root.get());

    return viewer.run();
}