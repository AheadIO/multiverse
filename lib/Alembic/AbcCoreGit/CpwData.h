/*****************************************************************************/
/*  multiverse - a next generation storage back-end for Alembic              */
/*  Copyright (C) 2015 J CUBE Inc. Tokyo, Japan. All Rights Reserved.        */
/*                                                                           */
/*  This program is free software: you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation, either version 3 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*                                                                           */
/*    J CUBE Inc.                                                            */
/*    6F Azabu Green Terrace                                                 */
/*    3-20-1 Minami-Azabu, Minato-ku, Tokyo                                  */
/*    info@-jcube.jp                                                         */
/*****************************************************************************/

#ifndef _Alembic_AbcCoreGit_CpwData_h_
#define _Alembic_AbcCoreGit_CpwData_h_

#include <Alembic/AbcCoreGit/Foundation.h>
#include <Alembic/AbcCoreGit/MetaDataMap.h>
#include <Alembic/AbcCoreGit/Git.h>

namespace Alembic {
namespace AbcCoreGit {
namespace ALEMBIC_VERSION_NS {

class AwImpl;

// data class owned by CpwImpl, or OwImpl if it is a "top" object
// it owns and makes child properties as well as the group GitGroupPtr
// when necessary
class CpwData : public Alembic::Util::enable_shared_from_this<CpwData>
{
public:

    CpwData( const std::string & iName, GitGroupPtr iGroup );

    ~CpwData();

    size_t getNumProperties();

    const AbcA::PropertyHeader & getPropertyHeader( size_t i );

    const AbcA::PropertyHeader * getPropertyHeader( const std::string &iName );

    AbcA::BasePropertyWriterPtr getProperty( const std::string & iName );

    AbcA::ScalarPropertyWriterPtr
    createScalarProperty( AbcA::CompoundPropertyWriterPtr iParent,
        const std::string & iName,
        const AbcA::MetaData & iMetaData,
        const AbcA::DataType & iDataType,
        uint32_t iTimeSamplingIndex );

    AbcA::ArrayPropertyWriterPtr
    createArrayProperty( AbcA::CompoundPropertyWriterPtr iParent,
        const std::string & iName,
        const AbcA::MetaData & iMetaData,
        const AbcA::DataType & iDataType,
        uint32_t iTimeSamplingIndex );

    AbcA::CompoundPropertyWriterPtr
    createCompoundProperty( AbcA::CompoundPropertyWriterPtr iParent,
        const std::string & iName,
        const AbcA::MetaData & iMetaData );

    void writePropertyHeaders( MetaDataMapPtr iMetaDataMap );

    void fillHash( size_t iIndex, Util::uint64_t iHash0,
                   Util::uint64_t iHash1 );

    void computeHash( Util::SpookyHash & ioHash );

    const std::string& name() const               { return m_name; }
    std::string relPathname()                     { GitGroupPtr group = getGroup(); ABCA_ASSERT(group, "invalid group"); return group->relPathname(); }
    std::string absPathname()                     { GitGroupPtr group = getGroup(); ABCA_ASSERT(group, "invalid group"); return group->absPathname(); }

    void writeToDisk();

private:
    friend class CpwImpl;

    GitGroupPtr getGroup() { return m_group; }

    // The group corresponding to this property.
    GitGroupPtr m_group;

    // if m_group gets created it will be given this name
    std::string m_name;

    typedef std::map<std::string, WeakBpwPtr> MadeProperties;

    PropertyHeaderPtrs m_propertyHeaders;
    MadeProperties m_madeProperties;

    // child hashes
    std::vector< Util::uint64_t > m_hashes;
};

typedef Alembic::Util::shared_ptr<CpwData> CpwDataPtr;

} // End namespace ALEMBIC_VERSION_NS

using namespace ALEMBIC_VERSION_NS;

} // End namespace AbcCoreGit
} // End namespace Alembic

#endif
