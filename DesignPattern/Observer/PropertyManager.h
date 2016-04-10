/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
*/

#ifndef PROPERTYMANAGER
#define PROPERTYMANAGER 1

#include <osg/UserDataContainer>
#include <osg/ValueObject>
#include <osg/ImageSequence>
#include <osgGA/GUIEventHandler>

#include <osgPresentation/Export>

#include <sstream>

namespace SLCore
{

class PropertyManager : protected osg::Object
{
public:

    PropertyManager() {}
    PropertyManager(const PropertyManager& pm, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
        osg::Object(pm,copyop) {}

    META_Object(SLCore, PropertyManager)

    /** Convenience method that casts the named UserObject to osg::TemplateValueObject<T> and gets the value.
        * To use this template method you need to include the osg/ValueObject header.*/
    template<typename T>
    bool getProperty(const std::string& name, T& value) const
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return getUserValue(name, value);
    }

    /** Convenience method that creates the osg::TemplateValueObject<T> to store the
        * specified value and adds it as a named UserObject.
        * To use this template method you need to include the osg/ValueObject header. */
    template<typename T>
    void setProperty(const std::string& name, const T& value)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return setUserValue(name, value);
    }

    int ref() const { return osg::Referenced::ref(); }
    int unref() const { return osg::Referenced::unref(); }

protected:

    mutable OpenThreads::Mutex _mutex;

};

extern OSGPRESENTATION_EXPORT const osg::Object* getUserObject(const osg::NodePath& nodepath, const std::string& name);

template<typename T>
bool getUserValue(const osg::NodePath& nodepath, const std::string& name, T& value)
{
    typedef osg::TemplateValueObject<T> UserValueObject;
    const osg::Object* object = getUserObject(nodepath, name);
    const UserValueObject* uvo = dynamic_cast<const UserValueObject*>(object);

    if (uvo)
    {
        value = uvo->getValue();
        return true;
    }
    else
    {
        return false;
    }
}

extern OSGPRESENTATION_EXPORT bool containsPropertyReference(const std::string& str);

struct PropertyReader
{
    PropertyReader(const osg::NodePath& nodePath, const std::string& str):
        _errorGenerated(false),
        _nodePath(nodePath),
        _sstream(str) {}

    template<typename T>
    bool read(T& value)
    {
        // skip white space.
        while(!_sstream.fail() && _sstream.peek()==' ') _sstream.ignore();

        // check to see if a &propertyName is used.
        if (_sstream.peek()=='$')
        {
            std::string propertyName;
            _sstream.ignore(1);
            _sstream >> propertyName;
            OSG_NOTICE<<"Reading propertyName="<<propertyName<<std::endl;
            if (!_sstream.fail() && !propertyName.empty()) return getUserValue(_nodePath, propertyName, value);
            else return false;
        }
        else
        {
            _sstream >> value;
            OSG_NOTICE<<"Reading value="<<value<<std::endl;
            return !_sstream.fail();
        }
    }

    template<typename T>
    PropertyReader& operator>>( T& value ) { if (!read(value)) _errorGenerated=true; return *this; }

    bool ok() { return !_sstream.fail() && !_errorGenerated; }
    bool fail() { return _sstream.fail() || _errorGenerated; }

    bool                _errorGenerated;
    osg::NodePath       _nodePath;
    std::istringstream  _sstream;
};

#endif
