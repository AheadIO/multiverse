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

#ifndef _Alembic_AbcCoreGit_ApwImpl_h_
#define _Alembic_AbcCoreGit_ApwImpl_h_

#include <Alembic/AbcCoreGit/Foundation.h>
#include <Alembic/AbcCoreGit/WrittenSampleMap.h>
#include <Alembic/AbcCoreGit/SampleStore.h>
#include <Alembic/AbcCoreGit/CpwImpl.h>
#include <Alembic/AbcCoreGit/Git.h>

namespace Alembic {
namespace AbcCoreGit {
namespace ALEMBIC_VERSION_NS {

//-*****************************************************************************
class ApwImpl
    : public AbcA::ArrayPropertyWriter
    , public Alembic::Util::enable_shared_from_this<ApwImpl>
{
protected:
    friend class CpwData;

    //-*************************************************************************
    ApwImpl( AbcA::CompoundPropertyWriterPtr iParent,
             GitGroupPtr iGroup,
             PropertyHeaderPtr iHeader,
             size_t iIndex );

    virtual AbcA::ArrayPropertyWriterPtr asArrayPtr();

public:
    virtual ~ApwImpl();

    // ArrayPropertyWriter overrides
    virtual void setSample( const AbcA::ArraySample & iSamp );
    virtual void setFromPreviousSample();
    virtual size_t getNumSamples();
    virtual void setTimeSamplingIndex( Util::uint32_t iIndex );

    // BasePropertyWriter overrides
    virtual const AbcA::PropertyHeader & getHeader() const;
    virtual AbcA::ObjectWriterPtr getObject();
    virtual AbcA::CompoundPropertyWriterPtr getParent();

    CpwImplPtr getTParent() const;

    std::string repr(bool extended=false) const;

    const std::string& name() const               { return m_header->name(); }
    std::string relPathname() const;
    std::string absPathname() const;

    void writeToDisk();

    Alembic::Util::shared_ptr< AwImpl > getArchiveImpl() const;

protected:
    // Previous written array sample identifier!
    WrittenSampleIDPtr m_previousWrittenSampleID;

private:
    void ensureSampleStore();
    void ensureSampleStore(const AbcA::Dimensions &iDims);

    // The parent compound property writer.
    AbcA::CompoundPropertyWriterPtr m_parent;

    // The header which defines this property.
    // And extra data the parent needs to eventually write out
    PropertyHeaderPtr m_header;

    // for accumulating our hierarchical hash
    Util::Digest m_hash;

    Alembic::Util::auto_ptr<AbstractTypedSampleStore> m_store;

    GitGroupPtr m_group;

    AbcA::Dimensions m_dims;

    size_t m_index;

    bool m_written;
};

} // End namespace ALEMBIC_VERSION_NS

using namespace ALEMBIC_VERSION_NS;

} // End namespace AbcCoreGit
} // End namespace Alembic

#endif
