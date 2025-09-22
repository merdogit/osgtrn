#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

int main(int argc, char **argv)
{
    // Load the image file
    osg::ref_ptr<osg::Image> image =
        osgDB::readImageFile("/home/murate/Documents/SwTrn/OsgTrn/OpenSceneGraph-Data/ground-mud-puddle-4096x4096.jpg");

    if (!image)
    {
        std::cerr << "Failed to load image!" << std::endl;
        return 1;
    }

    // Create texture from image
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage(image);

    // Create a textured quad (plane)
    osg::ref_ptr<osg::Geometry> quad = osg::createTexturedQuadGeometry(
        osg::Vec3(-5.0f, 0.0f, -5.0f), // corner position
        osg::Vec3(10.0f, 0.0f, 0.0f),  // width vector
        osg::Vec3(0.0f, 0.0f, 10.0f)   // height vector
    );

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(quad);

    // Apply texture to quad
    osg::StateSet *stateSet = geode->getOrCreateStateSet();
    stateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

    // Path to OSG sample data (Debian installs here)
    std::string dataPath = "/home/murate/Documents/SwTrn/OsgTrn/OpenSceneGraph-Data/F-14-low-poly-osg.ac";

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(dataPath);
    if (!model)
    {
        std::cerr << "Failed to load model from: " << dataPath << std::endl;
        return 1;
    }

    // Create scene root
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild(geode);
    root->addChild(model);

    // Viewer
    osgViewer::Viewer viewer;
    viewer.setSceneData(root);

    return viewer.run();
}