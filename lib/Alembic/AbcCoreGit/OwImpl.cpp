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

#include <Alembic/AbcCoreGit/OwData.h>
#include <Alembic/AbcCoreGit/OwImpl.h>
#include <Alembic/AbcCoreGit/AwImpl.h>
#include <Alembic/AbcCoreGit/CpwImpl.h>
#include <Alembic/AbcCoreGit/Utils.h>

namespace Alembic {
namespace AbcCoreGit {
namespace ALEMBIC_VERSION_NS {

OwImpl::OwImpl( AbcA::ArchiveWriterPtr iArchive,
                OwDataPtr iData,
                const AbcA::MetaData & iMetaData )
    : m_archive( iArchive )
    , m_header( new AbcA::ObjectHeader( "ABC", "/", iMetaData ) )
    , m_data( iData )
    , m_index( 0 )
{
    if (iData)
        TRACE("OwImpl::OwImpl(iArchive" << ", group:'" << iData->getGroup()->fullname() << "')");
    else
        TRACE("OwImpl::OwImpl(iArchive" << ", group:NULL)");

    ABCA_ASSERT( m_archive, "Invalid archive" );
    ABCA_ASSERT( m_data, "Invalid data" );
}

//-*****************************************************************************
OwImpl::OwImpl( AbcA::ObjectWriterPtr iParent,
                GitGroupPtr iGroup,
                ObjectHeaderPtr iHeader,
                size_t iIndex )
  : m_parent( iParent )
  , m_header( iHeader )
  , m_index( iIndex )
{
    // Check validity of all inputs.
    ABCA_ASSERT( m_parent, "Invalid parent" );
    ABCA_ASSERT( m_header, "Invalid header" );
    ABCA_ASSERT( iGroup, "Invalid group" );

    TRACE("OwImpl::OwImpl(parent:" << CONCRETE_OWPTR(m_parent)->repr() << ", group:'" << iGroup->fullname() << "', objHeader:'" << m_header->getName() << "', index:" << m_index << ")");

    m_archive = m_parent->getArchive();
    ABCA_ASSERT( m_archive, "Invalid archive" );

    m_data.reset( new OwData( iGroup, m_header ) );
}

//-*****************************************************************************
OwImpl::~OwImpl()
{
    // The archive is responsible for writing the MetaData
    if ( m_parent )
    {
        MetaDataMapPtr mdMap = Alembic::Util::dynamic_pointer_cast<
            AwImpl, AbcA::ArchiveWriter >( m_archive )->getMetaDataMap();

        Util::SpookyHash hash;
        hash.Init(0, 0);
        m_data->writeHeaders( mdMap, hash );

        // writeHeaders bakes in the child hashes and the data hash
        // but we still need to bake in the name and MetaData
        std::string metaDataStr = m_header->getMetaData().serialize();
        if ( !metaDataStr.empty() )
        {
            hash.Update( &( metaDataStr[0] ), metaDataStr.size() );
        }

        hash.Update( &( m_header->getName()[0] ), m_header->getName().size() );
        Util::uint64_t hash0, hash1;
        hash.Final( &hash0, &hash1 );

        Util::shared_ptr< OwImpl > parent =
            Alembic::Util::dynamic_pointer_cast< OwImpl,
                AbcA::ObjectWriter > ( m_parent );
        parent->fillHash( m_index, hash0, hash1 );
    }

    writeToDisk();
}

//-*****************************************************************************
const AbcA::ObjectHeader & OwImpl::getHeader() const
{
    ABCA_ASSERT( m_header, "Invalid header" );
    return *m_header;
}

//-*****************************************************************************
AbcA::ArchiveWriterPtr OwImpl::getArchive()
{
    return m_archive;
}

//-*****************************************************************************
AbcA::ObjectWriterPtr OwImpl::getParent()
{
    return m_parent;
}

//-*****************************************************************************
AbcA::CompoundPropertyWriterPtr OwImpl::getProperties()
{
    return m_data->getProperties( asObjectPtr() );
}

//-*****************************************************************************
size_t OwImpl::getNumChildren()
{
    return m_data->getNumChildren();
}

//-*****************************************************************************
const AbcA::ObjectHeader & OwImpl::getChildHeader( size_t i )
{
    return m_data->getChildHeader( i );
}

const AbcA::ObjectHeader * OwImpl::getChildHeader( const std::string &iName )
{
    return m_data->getChildHeader( iName );
}

//-*****************************************************************************
AbcA::ObjectWriterPtr OwImpl::getChild( const std::string &iName )
{
    return m_data->getChild( iName );
}

//-*****************************************************************************
AbcA::ObjectWriterPtr OwImpl::createChild( const AbcA::ObjectHeader &iHeader )
{
    return m_data->createChild( asObjectPtr(), m_header->getFullName(),
                                iHeader );
}

//-*****************************************************************************
AbcA::ObjectWriterPtr OwImpl::asObjectPtr()
{
    return shared_from_this();
}

void OwImpl::fillHash( size_t iIndex, Util::uint64_t iHash0,
                       Util::uint64_t iHash1 )
{
    m_data->fillHash( iIndex, iHash0, iHash1 );
}

OwImplPtr OwImpl::getTParent() const
{
    return CONCRETE_OWPTR(m_parent);
}

std::string OwImpl::repr(bool extended) const
{
    std::ostringstream ss;
    if (extended)
    {
        if (m_parent)
        {
            OwImplPtr parentPtr = getTParent();

            ss << "<OwImpl(parent:" << parentPtr->repr() << ", objHeader:'" << m_header->getName() << "')>";
        } else
        {
            ss << "<OwImpl(TOP, " << ", objHeader:'" << m_header->getName() << "')>";
        }
    } else
    {
        ss << "'" << m_header->getName() << "'";
    }
    return ss.str();
}

void OwImpl::writeToDisk()
{
    TRACE("OwImpl::writeToDisk() path:'" << absPathname() << "'");
    ABCA_ASSERT( m_data, "invalid OwData" );
    m_data->writeToDisk();
}

Alembic::Util::shared_ptr< AwImpl > OwImpl::getArchiveImpl() const
{
    Util::shared_ptr< AwImpl > archive =
       Alembic::Util::dynamic_pointer_cast< AwImpl,
        AbcA::ArchiveWriter > ( m_archive );
    return archive;
}


} // End namespace ALEMBIC_VERSION_NS
} // End namespace AbcCoreGit
} // End namespace Alembic
