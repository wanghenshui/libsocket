/**
 * Created by Jian Chen
 * @since  2015.12.12
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include <socket/dns/dns_record.hpp>
#include <socket/dns/dns_table.hpp>
#include <socket/ip/ip_address.hpp>
#include <chen/base/num.hpp>
#include <chen/base/map.hpp>
#include <limits>

using namespace chen;
using namespace chen::dns;

// -----------------------------------------------------------------------------
// RR
RR::RR(chen::dns::RRType type) : rrtype(type)
{
}

// encode & decode
void RR::encode(chen::dns::encoder &encoder) const
{
    // common
    encoder.pack(this->name, true);
    encoder.pack(this->rrtype);
    encoder.pack(this->rrclass);
    encoder.pack(this->ttl);

    // rdlength, placeholder here
    encoder.pack(static_cast<std::uint16_t>(0));
}

void RR::decode(chen::dns::decoder &decoder)
{
    decoder.unpack(this->name, true);
    decoder.unpack(this->rrtype);
    decoder.unpack(this->rrclass);
    decoder.unpack(this->ttl);
    decoder.unpack(this->rdlength);
}

void RR::decode(const chen::json::object &object)
{
    // unpack ttl only
    this->ttl = chen::map::find(object, "ttl", this->ttl);
}

std::shared_ptr<chen::dns::RR> RR::create(chen::dns::decoder &decoder)
{
    chen::dns::decoder detect(decoder);

    // detect type
    std::string unused;
    detect.unpack(unused, true);

    chen::dns::RRType rrtype = chen::dns::RRType::None;
    detect.unpack(rrtype);

    // build record
    std::shared_ptr<chen::dns::RR> record = table::build(rrtype);
    if (!record)
        record.reset(new chen::dns::Unknown);

    // decode it
    record->decode(decoder);

    return record;
}

// str
std::string RR::str(const std::string &sep) const
{
    std::string ret;

    ret += this->name + sep;
    ret += chen::num::str(this->ttl) + sep;
    ret += chen::dns::table::classToText(this->rrclass) + sep;
    ret += chen::dns::table::typeToText(this->rrtype);

    return ret;
}

// helper
void RR::adjust(chen::dns::encoder &encoder, std::size_t val) const
{
    if (val > std::numeric_limits<std::uint16_t>::max())
        throw error_size("dns: codec pack rdata size is overflow");

    auto rdlength = static_cast<std::uint16_t>(val);

    encoder.change(encoder.size() - rdlength - 2, static_cast<std::uint8_t>(rdlength >> 8 & 0xFF));
    encoder.change(encoder.size() - rdlength - 1, static_cast<std::uint8_t>(rdlength & 0xFF));
}

std::size_t RR::remain(const chen::dns::codec::iterator &beg,
                       const chen::dns::codec::iterator &cur) const
{
    auto used = std::distance(beg, cur);
    if (this->rdlength < used)
        throw error_size("dns: codec rdata is overflow");

    return static_cast<std::size_t>(this->rdlength - used);
}

std::string RR::escape(const std::string &text) const
{
    std::string ret("\"");

    // escape all non-printable characters
    for (auto ch : text)
    {
        if (std::isprint(ch))
            ret += ch;
        else
            ret += chen::str::format("\\%03u", static_cast<unsigned char>(ch));
    }

    ret += '"';

    return ret;
}

std::string RR::escape(std::size_t bits) const
{
    return "<<" + chen::num::str(bits) + "bits>>";
}


// -----------------------------------------------------------------------------
// Raw
Raw::Raw() : Raw(chen::dns::RRType::None)
{
}

Raw::Raw(chen::dns::RRType type) : RR(type)
{
}

std::string Raw::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->escape(this->rdata.size());
    return ret;
}

std::shared_ptr<chen::dns::RR> Raw::clone() const
{
    return std::make_shared<Raw>(*this);
}

void Raw::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->rdata, this->rdata.size());

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void Raw::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->rdata, this->rdlength);
}

void Raw::decode(const chen::json::object &object)
{
    RR::decode(object);

    this->rdata.clear();

    std::string rdata = chen::map::find(object, "rdata");
    std::copy(rdata.begin(), rdata.end(), this->rdata.begin());
}


// -----------------------------------------------------------------------------
// Unknown
std::shared_ptr<chen::dns::RR> Unknown::clone() const
{
    return std::make_shared<Unknown>(*this);
}


// -----------------------------------------------------------------------------
// A
A::A() : RR(chen::dns::RRType::A)
{
}

std::string A::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + chen::ip::address_v4::toString(this->address);
    return ret;
}

std::shared_ptr<chen::dns::RR> A::clone() const
{
    return std::make_shared<A>(*this);
}

void A::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->address);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void A::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->address);
}

void A::decode(const chen::json::object &object)
{
    RR::decode(object);

    // if address is string then use address_v4::toInteger
    auto address = chen::map::find(object, "address");

    if (address.isString())
        this->address = chen::ip::address_v4::toInteger(address);
    else
        this->address = address.toUnsigned();
}


// -----------------------------------------------------------------------------
// NS
NS::NS() : RR(chen::dns::RRType::NS)
{
}

std::string NS::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->nsdname;
    return ret;
}

std::shared_ptr<chen::dns::RR> NS::clone() const
{
    return std::make_shared<NS>(*this);
}

void NS::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->nsdname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NS::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->nsdname, true);
}

void NS::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->nsdname = chen::map::find(object, "nsdname", this->nsdname);
}


// -----------------------------------------------------------------------------
// MD
MD::MD() : RR(chen::dns::RRType::MD)
{
}

std::string MD::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->madname;
    return ret;
}

std::shared_ptr<chen::dns::RR> MD::clone() const
{
    return std::make_shared<MD>(*this);
}

void MD::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->madname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void MD::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->madname, true);
}

void MD::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->madname = chen::map::find(object, "madname", this->madname);
}


// -----------------------------------------------------------------------------
// MF
MF::MF() : RR(chen::dns::RRType::MF)
{
}

std::string MF::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += this->madname;
    return ret;
}

std::shared_ptr<chen::dns::RR> MF::clone() const
{
    return std::make_shared<MF>(*this);
}

void MF::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->madname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void MF::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->madname, true);
}

void MF::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->madname = chen::map::find(object, "madname", this->madname);
}


// -----------------------------------------------------------------------------
// CNAME
CNAME::CNAME() : RR(chen::dns::RRType::CNAME)
{
}

std::string CNAME::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->cname;
    return ret;
}

std::shared_ptr<chen::dns::RR> CNAME::clone() const
{
    return std::make_shared<CNAME>(*this);
}

void CNAME::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->cname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void CNAME::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->cname, true);
}

void CNAME::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->cname = chen::map::find(object, "cname", this->cname);
}


// -----------------------------------------------------------------------------
// SOA
SOA::SOA() : RR(chen::dns::RRType::SOA)
{
}

std::string SOA::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + this->mname;
    ret += " " + this->rname;  // use a space to seprate different fields
    ret += " " + chen::num::str(this->serial);
    ret += " " + chen::num::str(this->refresh);
    ret += " " + chen::num::str(this->retry);
    ret += " " + chen::num::str(this->expire);
    ret += " " + chen::num::str(this->minimum);

    return ret;
}

std::shared_ptr<chen::dns::RR> SOA::clone() const
{
    return std::make_shared<SOA>(*this);
}

void SOA::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->mname, true);
    encoder.pack(this->rname, true);
    encoder.pack(this->serial);
    encoder.pack(this->refresh);
    encoder.pack(this->retry);
    encoder.pack(this->expire);
    encoder.pack(this->minimum);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void SOA::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->mname, true);
    decoder.unpack(this->rname, true);
    decoder.unpack(this->serial);
    decoder.unpack(this->refresh);
    decoder.unpack(this->retry);
    decoder.unpack(this->expire);
    decoder.unpack(this->minimum);
}

void SOA::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->mname   = chen::map::find(object, "mname", this->mname);
    this->rname   = chen::map::find(object, "rname", this->rname);
    this->serial  = chen::map::find(object, "serial", this->serial);
    this->refresh = chen::map::find(object, "refresh", this->refresh);
    this->retry   = chen::map::find(object, "retry", this->retry);
    this->expire  = chen::map::find(object, "expire", this->expire);
    this->minimum = chen::map::find(object, "minimum", this->minimum);
}


// -----------------------------------------------------------------------------
// MB
MB::MB() : RR(chen::dns::RRType::MB)
{
}

std::string MB::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->madname;
    return ret;
}

std::shared_ptr<chen::dns::RR> MB::clone() const
{
    return std::make_shared<MB>(*this);
}

void MB::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->madname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void MB::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->madname, true);
}

void MB::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->madname = chen::map::find(object, "madname", this->madname);
}


// -----------------------------------------------------------------------------
// MG
MG::MG() : RR(chen::dns::RRType::MG)
{
}

std::string MG::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->mgmname;
    return ret;
}

std::shared_ptr<chen::dns::RR> MG::clone() const
{
    return std::make_shared<MG>(*this);
}

void MG::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->mgmname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void MG::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->mgmname, true);
}

void MG::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->mgmname = chen::map::find(object, "mgmname", this->mgmname);
}


// -----------------------------------------------------------------------------
// MR
MR::MR() : RR(chen::dns::RRType::MR)
{
}

std::string MR::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->newname;
    return ret;
}

std::shared_ptr<chen::dns::RR> MR::clone() const
{
    return std::make_shared<MR>(*this);
}

void MR::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->newname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void MR::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->newname, true);
}

void MR::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->newname = chen::map::find(object, "newname", this->newname);
}


// -----------------------------------------------------------------------------
// NUL
NUL::NUL() : Raw(chen::dns::RRType::NUL)
{
}

std::string NUL::str(const std::string &sep) const
{
    auto ret = Raw::str(sep);
    ret += sep + this->escape(this->anything.size());
    return ret;
}

std::shared_ptr<chen::dns::RR> NUL::clone() const
{
    return std::make_shared<NUL>(*this);
}

void NUL::encode(chen::dns::encoder &encoder) const
{
    // base
    Raw::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->anything, this->anything.size());

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NUL::decode(chen::dns::decoder &decoder)
{
    Raw::decode(decoder);
    decoder.unpack(this->anything, this->rdlength);
}

void NUL::decode(const chen::json::object &object)
{
    Raw::decode(object);

    this->anything.clear();

    std::string anything = chen::map::find(object, "anything", std::string());
    std::copy(anything.begin(), anything.end(), this->anything.begin());
}


// -----------------------------------------------------------------------------
// WKS
WKS::WKS() : RR(chen::dns::RRType::WKS)
{
}

std::string WKS::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->address);
    ret += " " + chen::num::str(this->protocol);
    ret += " " + this->escape(this->bitmap.size());

    return ret;
}

std::shared_ptr<chen::dns::RR> WKS::clone() const
{
    return std::make_shared<WKS>(*this);
}

void WKS::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->address);
    encoder.pack(this->protocol);
    encoder.pack(this->bitmap, this->bitmap.size());

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void WKS::decode(chen::dns::decoder &decoder)
{
    auto tmp = decoder.cur();

    RR::decode(decoder);

    decoder.unpack(this->address);
    decoder.unpack(this->protocol);

    this->bitmap.clear();
    decoder.unpack(this->bitmap, this->remain(tmp, decoder.cur()));
}

void WKS::decode(const chen::json::object &object)
{
    RR::decode(object);

    this->address  = chen::map::find(object, "address", this->address);
    this->protocol = chen::map::find(object, "protocol", this->protocol);

    this->bitmap.clear();

    std::string bitmap = chen::map::find(object, "bitmap", std::string());
    std::copy(bitmap.begin(), bitmap.end(), this->bitmap.begin());
}


// -----------------------------------------------------------------------------
// PTR
PTR::PTR() : RR(chen::dns::RRType::PTR)
{
}

std::string PTR::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->ptrdname;
    return ret;
}

std::shared_ptr<chen::dns::RR> PTR::clone() const
{
    return std::make_shared<PTR>(*this);
}

void PTR::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->ptrdname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void PTR::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->ptrdname, true);
}

void PTR::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->ptrdname = chen::map::find(object, "ptrdname", this->ptrdname);
}


// -----------------------------------------------------------------------------
// HINFO
HINFO::HINFO() : RR(chen::dns::RRType::HINFO)
{
}

std::string HINFO::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->cpu;
    ret += " " + this->os;
    return ret;
}

std::shared_ptr<chen::dns::RR> HINFO::clone() const
{
    return std::make_shared<HINFO>(*this);
}

void HINFO::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->cpu, false);
    encoder.pack(this->os, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void HINFO::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->cpu, false);
    decoder.unpack(this->os, false);
}

void HINFO::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->cpu = chen::map::find(object, "cpu", this->cpu);
    this->os  = chen::map::find(object, "os", this->os);
}


// -----------------------------------------------------------------------------
// MINFO
MINFO::MINFO() : RR(chen::dns::RRType::MINFO)
{
}

std::string MINFO::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->rmailbx;
    ret += " " + this->emailbx;
    return ret;
}

std::shared_ptr<chen::dns::RR> MINFO::clone() const
{
    return std::make_shared<MINFO>(*this);
}

void MINFO::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->rmailbx, true);
    encoder.pack(this->emailbx, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void MINFO::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->rmailbx, true);
    decoder.unpack(this->emailbx, true);
}

void MINFO::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->rmailbx = chen::map::find(object, "rmailbx", this->rmailbx);
    this->emailbx = chen::map::find(object, "emailbx", this->emailbx);
}


// -----------------------------------------------------------------------------
// MX
MX::MX() : RR(chen::dns::RRType::MX)
{
}

std::string MX::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + chen::num::str(this->preference);
    ret += " " + this->exchange;
    return ret;
}

std::shared_ptr<chen::dns::RR> MX::clone() const
{
    return std::make_shared<MX>(*this);
}

void MX::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->preference);
    encoder.pack(this->exchange, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void MX::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->preference);
    decoder.unpack(this->exchange, true);
}

void MX::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->preference = chen::map::find(object, "preference", this->preference);
    this->exchange   = chen::map::find(object, "exchange", this->exchange);
}


// -----------------------------------------------------------------------------
// TXT
TXT::TXT() : RR(chen::dns::RRType::TXT)
{
}

std::string TXT::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->escape(this->txt_data);
    return ret;
}

std::shared_ptr<chen::dns::RR> TXT::clone() const
{
    return std::make_shared<TXT>(*this);
}

void TXT::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->txt_data, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void TXT::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->txt_data, false);
}

void TXT::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->txt_data = chen::map::find(object, "txt_data", this->txt_data);
}


// -----------------------------------------------------------------------------
// RP
RP::RP() : RR(chen::dns::RRType::RP)
{
}

std::string RP::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->mbox_dname;
    ret += " " + this->txt_dname;
    return ret;
}

std::shared_ptr<chen::dns::RR> RP::clone() const
{
    return std::make_shared<RP>(*this);
}

void RP::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->mbox_dname, true);
    encoder.pack(this->txt_dname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void RP::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->mbox_dname, true);
    decoder.unpack(this->txt_dname, true);
}

void RP::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->mbox_dname = chen::map::find(object, "mbox_dname", this->mbox_dname);
    this->txt_dname  = chen::map::find(object, "txt_dname", this->txt_dname);
}


// -----------------------------------------------------------------------------
// AFSDB
AFSDB::AFSDB() : RR(chen::dns::RRType::AFSDB)
{
}

std::string AFSDB::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + chen::num::str(this->subtype);
    ret += " " + this->hostname;
    return ret;
}

std::shared_ptr<chen::dns::RR> AFSDB::clone() const
{
    return std::make_shared<AFSDB>(*this);
}

void AFSDB::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->subtype);
    encoder.pack(this->hostname, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void AFSDB::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->subtype);
    decoder.unpack(this->hostname, true);
}

void AFSDB::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->subtype  = chen::map::find(object, "subtype", this->subtype);
    this->hostname = chen::map::find(object, "hostname", this->hostname);
}


// -----------------------------------------------------------------------------
// X25
X25::X25() : RR(chen::dns::RRType::X25)
{
}

std::string X25::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->psdn_address;
    return ret;
}

std::shared_ptr<chen::dns::RR> X25::clone() const
{
    return std::make_shared<X25>(*this);
}

void X25::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->psdn_address, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void X25::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->psdn_address, false);
}

void X25::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->psdn_address = chen::map::find(object, "psdn_address", this->psdn_address);
}


// -----------------------------------------------------------------------------
// ISDN
ISDN::ISDN() : RR(chen::dns::RRType::ISDN)
{
}

std::string ISDN::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->isdn_address;
    ret += " " + this->sa;
    return ret;
}

std::shared_ptr<chen::dns::RR> ISDN::clone() const
{
    return std::make_shared<ISDN>(*this);
}

void ISDN::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->isdn_address, false);
    encoder.pack(this->sa, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void ISDN::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->isdn_address, false);
    decoder.unpack(this->sa, false);
}

void ISDN::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->isdn_address = chen::map::find(object, "isdn_address", this->isdn_address);
    this->sa           = chen::map::find(object, "sa", this->sa);
}


// -----------------------------------------------------------------------------
// RT
RT::RT() : RR(chen::dns::RRType::RT)
{
}

std::string RT::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + chen::num::str(this->preference);
    ret += " " + this->intermediate_host;
    return ret;
}

std::shared_ptr<chen::dns::RR> RT::clone() const
{
    return std::make_shared<RT>(*this);
}

void RT::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->preference);
    encoder.pack(this->intermediate_host, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void RT::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->preference);
    decoder.unpack(this->intermediate_host, true);
}

void RT::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->preference        = chen::map::find(object, "preference", this->preference);
    this->intermediate_host = chen::map::find(object, "intermediate_host", this->intermediate_host);
}


// -----------------------------------------------------------------------------
// NSAP
NSAP::NSAP() : RR(chen::dns::RRType::NSAP)
{
}

std::string NSAP::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->nsap;
    return ret;
}

std::shared_ptr<chen::dns::RR> NSAP::clone() const
{
    return std::make_shared<NSAP>(*this);
}

void NSAP::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->nsap, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NSAP::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->nsap, false);
}

void NSAP::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->nsap = chen::map::find(object, "nsap", this->nsap);
}


// -----------------------------------------------------------------------------
// NSAPPTR
NSAPPTR::NSAPPTR() : RR(chen::dns::RRType::NSAPPTR)
{
}

std::string NSAPPTR::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->owner;
    return ret;
}

std::shared_ptr<chen::dns::RR> NSAPPTR::clone() const
{
    return std::make_shared<NSAPPTR>(*this);
}

void NSAPPTR::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->owner, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NSAPPTR::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->owner, true);
}

void NSAPPTR::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->owner = chen::map::find(object, "owner", this->owner);
}


// -----------------------------------------------------------------------------
// SIG
SIG::SIG() : RR(chen::dns::RRType::SIG)
{
}

std::string SIG::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->type_covered);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + chen::num::str(this->labels);
    ret += " " + chen::num::str(this->original);
    ret += " " + chen::num::str(this->expiration);
    ret += " " + chen::num::str(this->inception);
    ret += " " + chen::num::str(this->key_tag);
    ret += " " + this->signer;
    ret += " " + this->signature;

    return ret;
}

std::shared_ptr<chen::dns::RR> SIG::clone() const
{
    return std::make_shared<SIG>(*this);
}

void SIG::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->type_covered);
    encoder.pack(this->algorithm);
    encoder.pack(this->labels);
    encoder.pack(this->original);
    encoder.pack(this->expiration);
    encoder.pack(this->inception);
    encoder.pack(this->key_tag);
    encoder.pack(this->signer, true);
    encoder.pack(this->signature, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void SIG::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->type_covered);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->labels);
    decoder.unpack(this->original);
    decoder.unpack(this->expiration);
    decoder.unpack(this->inception);
    decoder.unpack(this->key_tag);
    decoder.unpack(this->signer, true);
    decoder.unpack(this->signature, false);
}

void SIG::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->type_covered = chen::map::find(object, "type_covered", this->type_covered);
    this->algorithm    = chen::map::find(object, "algorithm", this->algorithm);
    this->labels       = chen::map::find(object, "labels", this->labels);
    this->original     = chen::map::find(object, "original", this->original);
    this->expiration   = chen::map::find(object, "expiration", this->expiration);
    this->inception    = chen::map::find(object, "inception", this->inception);
    this->key_tag      = chen::map::find(object, "key_tag", this->key_tag);
    this->signer       = chen::map::find(object, "signer", this->signer);
    this->signature    = chen::map::find(object, "signature", this->signature);
}


// -----------------------------------------------------------------------------
// KEY
KEY::KEY() : RR(chen::dns::RRType::KEY)
{
}

std::string KEY::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->flags);
    ret += " " + chen::num::str(this->protocol);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + this->publickey;

    return ret;
}

std::shared_ptr<chen::dns::RR> KEY::clone() const
{
    return std::make_shared<KEY>(*this);
}

void KEY::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->flags);
    encoder.pack(this->protocol);
    encoder.pack(this->algorithm);
    encoder.pack(this->publickey, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void KEY::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->flags);
    decoder.unpack(this->protocol);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->publickey, false);
}

void KEY::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->flags     = chen::map::find(object, "flags", this->flags);
    this->protocol  = chen::map::find(object, "protocol", this->protocol);
    this->algorithm = chen::map::find(object, "algorithm", this->algorithm);
    this->publickey = chen::map::find(object, "publickey", this->publickey);
}


// -----------------------------------------------------------------------------
// PX
PX::PX() : RR(chen::dns::RRType::PX)
{
}

std::string PX::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->preference);
    ret += " " + this->map822;
    ret += " " + this->mapx400;

    return ret;
}

std::shared_ptr<chen::dns::RR> PX::clone() const
{
    return std::make_shared<PX>(*this);
}

void PX::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->preference);
    encoder.pack(this->map822, true);
    encoder.pack(this->mapx400, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void PX::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->preference);
    decoder.unpack(this->map822, true);
    decoder.unpack(this->mapx400, true);
}

void PX::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->preference = chen::map::find(object, "preference", this->preference);
    this->map822     = chen::map::find(object, "map822", this->map822);
    this->mapx400    = chen::map::find(object, "mapx400", this->mapx400);
}


// -----------------------------------------------------------------------------
// GPOS
GPOS::GPOS() : RR(chen::dns::RRType::GPOS)
{
}

std::string GPOS::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + this->longitude;
    ret += " " + this->latitude;
    ret += " " + this->altitude;

    return ret;
}

std::shared_ptr<chen::dns::RR> GPOS::clone() const
{
    return std::make_shared<GPOS>(*this);
}

void GPOS::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->longitude, false);
    encoder.pack(this->latitude, false);
    encoder.pack(this->altitude, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void GPOS::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->longitude, false);
    decoder.unpack(this->latitude, false);
    decoder.unpack(this->altitude, false);
}

void GPOS::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->longitude = chen::map::find(object, "longitude", this->longitude);
    this->latitude  = chen::map::find(object, "latitude", this->latitude);
    this->altitude  = chen::map::find(object, "altitude", this->altitude);
}


// -----------------------------------------------------------------------------
// AAAA
AAAA::AAAA() : RR(chen::dns::RRType::AAAA)
{
}

std::string AAAA::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + chen::ip::address_v6::toString(this->address);
    return ret;
}

std::shared_ptr<chen::dns::RR> AAAA::clone() const
{
    return std::make_shared<AAAA>(*this);
}

void AAAA::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->address);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void AAAA::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->address);
}

void AAAA::decode(const chen::json::object &object)
{
    RR::decode(object);
    auto address = chen::map::find(object, "address");
    this->address = chen::ip::address_v6::toBytes(address);
}


// -----------------------------------------------------------------------------
// LOC
LOC::LOC() : RR(chen::dns::RRType::LOC)
{
}

std::string LOC::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->version);
    ret += " " + chen::num::str(this->size);
    ret += " " + chen::num::str(this->horiz_pre);
    ret += " " + chen::num::str(this->vert_pre);
    ret += " " + chen::num::str(this->longitude);
    ret += " " + chen::num::str(this->latitude);
    ret += " " + chen::num::str(this->altitude);

    return ret;
}

std::shared_ptr<chen::dns::RR> LOC::clone() const
{
    return std::make_shared<LOC>(*this);
}

void LOC::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->version);
    encoder.pack(this->size);
    encoder.pack(this->horiz_pre);
    encoder.pack(this->vert_pre);
    encoder.pack(this->longitude);
    encoder.pack(this->latitude);
    encoder.pack(this->altitude);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void LOC::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->version);
    decoder.unpack(this->size);
    decoder.unpack(this->horiz_pre);
    decoder.unpack(this->vert_pre);
    decoder.unpack(this->longitude);
    decoder.unpack(this->latitude);
    decoder.unpack(this->altitude);
}

void LOC::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->version   = chen::map::find(object, "version", this->version);
    this->size      = chen::map::find(object, "size", this->size);
    this->horiz_pre = chen::map::find(object, "horiz_pre", this->horiz_pre);
    this->vert_pre  = chen::map::find(object, "vert_pre", this->vert_pre);
    this->longitude = chen::map::find(object, "longitude", this->longitude);
    this->latitude  = chen::map::find(object, "latitude", this->latitude);
    this->altitude  = chen::map::find(object, "altitude", this->altitude);
}


// -----------------------------------------------------------------------------
// NXT
NXT::NXT() : RR(chen::dns::RRType::NXT)
{
}

std::string NXT::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->next_domain;
    ret += " " + this->escape(this->type_bitmap.size());
    return ret;
}

std::shared_ptr<chen::dns::RR> NXT::clone() const
{
    return std::make_shared<NXT>(*this);
}

void NXT::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->next_domain, true);
    encoder.pack(this->type_bitmap, this->type_bitmap.size());

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NXT::decode(chen::dns::decoder &decoder)
{
    auto tmp = decoder.cur();

    RR::decode(decoder);
    decoder.unpack(this->next_domain, true);

    this->type_bitmap.clear();
    decoder.unpack(this->type_bitmap, this->remain(tmp, decoder.cur()));
}

void NXT::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->next_domain = chen::map::find(object, "next_domain", this->next_domain);

    this->type_bitmap.clear();

    std::string type_bitmap = chen::map::find(object, "type_bitmap", std::string());
    std::copy(type_bitmap.begin(), type_bitmap.end(), this->type_bitmap.begin());
}


// -----------------------------------------------------------------------------
// EID
EID::EID() : RR(chen::dns::RRType::EID)
{
}

std::string EID::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->endpoint;
    return ret;
}

std::shared_ptr<chen::dns::RR> EID::clone() const
{
    return std::make_shared<EID>(*this);
}

void EID::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->endpoint, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void EID::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->endpoint, false);
}

void EID::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->endpoint = chen::map::find(object, "endpoint", this->endpoint);
}


// -----------------------------------------------------------------------------
// NIMLOC
NIMLOC::NIMLOC() : RR(chen::dns::RRType::NIMLOC)
{
}

std::string NIMLOC::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->locator;
    return ret;
}

std::shared_ptr<chen::dns::RR> NIMLOC::clone() const
{
    return std::make_shared<NIMLOC>(*this);
}

void NIMLOC::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->locator, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NIMLOC::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->locator, false);
}

void NIMLOC::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->locator = chen::map::find(object, "locator", this->locator);
}


// -----------------------------------------------------------------------------
// SRV
SRV::SRV() : RR(chen::dns::RRType::SRV)
{
}

std::string SRV::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->priority);
    ret += " " + chen::num::str(this->weight);
    ret += " " + chen::num::str(this->port);
    ret += " " + this->target;

    return ret;
}

std::shared_ptr<chen::dns::RR> SRV::clone() const
{
    return std::make_shared<SRV>(*this);
}

void SRV::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->priority);
    encoder.pack(this->weight);
    encoder.pack(this->port);
    encoder.pack(this->target, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void SRV::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->priority);
    decoder.unpack(this->weight);
    decoder.unpack(this->port);
    decoder.unpack(this->target, true);
}

void SRV::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->priority = chen::map::find(object, "priority", this->priority);
    this->weight   = chen::map::find(object, "weight", this->weight);
    this->port     = chen::map::find(object, "port", this->port);
    this->target   = chen::map::find(object, "target", this->target);
}


// -----------------------------------------------------------------------------
// ATMA
ATMA::ATMA() : RR(chen::dns::RRType::ATMA)
{
}

std::string ATMA::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->format);
    ret += " " + this->address;

    return ret;
}

std::shared_ptr<chen::dns::RR> ATMA::clone() const
{
    return std::make_shared<ATMA>(*this);
}

void ATMA::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->format);
    encoder.pack(this->address, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void ATMA::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->format);
    decoder.unpack(this->address, false);
}

void ATMA::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->format  = chen::map::find(object, "format", this->format);
    this->address = chen::map::find(object, "address", this->address);
}


// -----------------------------------------------------------------------------
// NAPTR
NAPTR::NAPTR() : RR(chen::dns::RRType::NAPTR)
{
}

std::string NAPTR::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->order);
    ret += " " + chen::num::str(this->preference);
    ret += " " + this->flags;
    ret += " " + this->services;
    ret += " " + this->regexp;
    ret += " " + this->replacement;

    return ret;
}

std::shared_ptr<chen::dns::RR> NAPTR::clone() const
{
    return std::make_shared<NAPTR>(*this);
}

void NAPTR::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->order);
    encoder.pack(this->preference);
    encoder.pack(this->flags, false);
    encoder.pack(this->services, false);
    encoder.pack(this->regexp, false);
    encoder.pack(this->replacement, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NAPTR::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->order);
    decoder.unpack(this->preference);
    decoder.unpack(this->flags, false);
    decoder.unpack(this->services, false);
    decoder.unpack(this->regexp, false);
    decoder.unpack(this->replacement, true);
}

void NAPTR::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->order       = chen::map::find(object, "order", this->order);
    this->preference  = chen::map::find(object, "preference", this->preference);
    this->flags       = chen::map::find(object, "flags", this->flags);
    this->services    = chen::map::find(object, "services", this->services);
    this->regexp      = chen::map::find(object, "regexp", this->regexp);
    this->replacement = chen::map::find(object, "replacement", this->replacement);
}


// -----------------------------------------------------------------------------
// KX
KX::KX() : RR(chen::dns::RRType::KX)
{
}

std::string KX::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->preference);
    ret += " " + this->exchanger;

    return ret;
}

std::shared_ptr<chen::dns::RR> KX::clone() const
{
    return std::make_shared<KX>(*this);
}

void KX::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->preference);
    encoder.pack(this->exchanger, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void KX::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->preference);
    decoder.unpack(this->exchanger, true);
}

void KX::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->preference = chen::map::find(object, "preference", this->preference);
    this->exchanger  = chen::map::find(object, "exchanger", this->exchanger);
}


// -----------------------------------------------------------------------------
// CERT
CERT::CERT() : RR(chen::dns::RRType::CERT)
{
}

std::string CERT::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->type);
    ret += " " + chen::num::str(this->key_tag);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + this->certificate;

    return ret;
}

std::shared_ptr<chen::dns::RR> CERT::clone() const
{
    return std::make_shared<CERT>(*this);
}

void CERT::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->type);
    encoder.pack(this->key_tag);
    encoder.pack(this->algorithm);
    encoder.pack(this->certificate, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void CERT::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->type);
    decoder.unpack(this->key_tag);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->certificate, false);
}

void CERT::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->type        = chen::map::find(object, "type", this->type);
    this->key_tag     = chen::map::find(object, "key_tag", this->key_tag);
    this->algorithm   = chen::map::find(object, "algorithm", this->algorithm);
    this->certificate = chen::map::find(object, "certificate", this->certificate);
}


// -----------------------------------------------------------------------------
// A6
A6::A6() : RR(chen::dns::RRType::A6)
{
}

std::string A6::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->prefix);
    ret += " " + this->escape(this->suffix.size());
    ret += " " + this->prefix_name;

    return ret;
}

std::shared_ptr<chen::dns::RR> A6::clone() const
{
    return std::make_shared<A6>(*this);
}

void A6::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->prefix);
    encoder.pack(this->suffix, this->suffix.size());
    encoder.pack(this->prefix_name, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void A6::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->prefix);

    this->suffix.clear();
    decoder.unpack(this->suffix, static_cast<std::size_t>(128 - this->prefix));

    decoder.unpack(this->prefix_name, true);
}

void A6::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->prefix = chen::map::find(object, "prefix", this->prefix);

    this->suffix.clear();

    std::string suffix = chen::map::find(object, "suffix", std::string());
    std::copy(suffix.begin(), suffix.end(), this->suffix.begin());

    this->prefix_name = chen::map::find(object, "prefix_name", this->prefix_name);
}


// -----------------------------------------------------------------------------
// DNAME
DNAME::DNAME() : RR(chen::dns::RRType::DNAME)
{
}

std::string DNAME::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->target;
    return ret;
}

std::shared_ptr<chen::dns::RR> DNAME::clone() const
{
    return std::make_shared<DNAME>(*this);
}

void DNAME::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->target, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void DNAME::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->target, true);
}

void DNAME::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->target = chen::map::find(object, "target", this->target);
}


// -----------------------------------------------------------------------------
// SINK
SINK::SINK() : RR(chen::dns::RRType::SINK)
{
}

std::string SINK::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->coding);
    ret += " " + chen::num::str(this->subcoding);
    ret += " " + this->escape(this->sdata.size());

    return ret;
}

std::shared_ptr<chen::dns::RR> SINK::clone() const
{
    return std::make_shared<SINK>(*this);
}

void SINK::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->coding);
    encoder.pack(this->subcoding);
    encoder.pack(this->sdata, this->sdata.size());

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void SINK::decode(chen::dns::decoder &decoder)
{
    auto tmp = decoder.cur();

    RR::decode(decoder);
    decoder.unpack(this->coding);
    decoder.unpack(this->subcoding);

    this->sdata.clear();
    decoder.unpack(this->sdata, this->remain(tmp, decoder.cur()));
}

void SINK::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->coding    = chen::map::find(object, "coding", this->coding);
    this->subcoding = chen::map::find(object, "subcoding", this->subcoding);

    this->sdata.clear();

    std::string sdata = chen::map::find(object, "sdata", std::string());
    std::copy(sdata.begin(), sdata.end(), this->sdata.begin());
}


// -----------------------------------------------------------------------------
// OPT
OPT::OPT() : Raw(chen::dns::RRType::OPT)
{
}

std::shared_ptr<chen::dns::RR> OPT::clone() const
{
    return std::make_shared<OPT>(*this);
}


// -----------------------------------------------------------------------------
// DS
DS::DS() : RR(chen::dns::RRType::DS)
{
}

std::string DS::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->key_tag);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + chen::num::str(this->digest_type);
    ret += " " + this->digest;

    return ret;
}

std::shared_ptr<chen::dns::RR> DS::clone() const
{
    return std::make_shared<DS>(*this);
}

void DS::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->key_tag);
    encoder.pack(this->algorithm);
    encoder.pack(this->digest_type);
    encoder.pack(this->digest, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void DS::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->key_tag);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->digest_type);
    decoder.unpack(this->digest, false);
}

void DS::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->key_tag     = chen::map::find(object, "key_tag", this->key_tag);
    this->algorithm   = chen::map::find(object, "algorithm", this->algorithm);
    this->digest_type = chen::map::find(object, "digest_type", this->digest_type);
    this->digest      = chen::map::find(object, "digest", this->digest);
}


// -----------------------------------------------------------------------------
// SSHFP
SSHFP::SSHFP() : RR(chen::dns::RRType::SSHFP)
{
}

std::string SSHFP::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->algorithm);
    ret += " " + chen::num::str(this->fptype);
    ret += " " + this->fingerprint;

    return ret;
}

std::shared_ptr<chen::dns::RR> SSHFP::clone() const
{
    return std::make_shared<SSHFP>(*this);
}

void SSHFP::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->algorithm);
    encoder.pack(this->fptype);
    encoder.pack(this->fingerprint, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void SSHFP::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->fptype);
    decoder.unpack(this->fingerprint, false);
}

void SSHFP::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->algorithm   = chen::map::find(object, "algorithm", this->algorithm);
    this->fptype      = chen::map::find(object, "fptype", this->fptype);
    this->fingerprint = chen::map::find(object, "fingerprint", this->fingerprint);
}


// -----------------------------------------------------------------------------
// IPSECKEY
IPSECKEY::IPSECKEY() : RR(chen::dns::RRType::IPSECKEY)
{
}

std::string IPSECKEY::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->precedence);
    ret += " " + chen::num::str(this->gateway_type);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + this->escape(this->gateway.size());
    ret += " " + this->publickey;

    return ret;
}

std::shared_ptr<chen::dns::RR> IPSECKEY::clone() const
{
    return std::make_shared<IPSECKEY>(*this);
}

void IPSECKEY::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->precedence);
    encoder.pack(this->gateway_type);
    encoder.pack(this->algorithm);

    switch (static_cast<GatewayType>(this->gateway_type))
    {
        case GatewayType::None:
            encoder.pack(this->gateway, 1);
            break;

        case GatewayType::IPv4:
            encoder.pack(this->gateway, 4);
            break;

        case GatewayType::IPv6:
            encoder.pack(this->gateway, 16);
            break;

        case GatewayType::Domain:
        {
            std::string domain(this->gateway.begin(), this->gateway.end());
            encoder.pack(domain, true);
        }
            break;
    }

    encoder.pack(this->publickey, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void IPSECKEY::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->precedence);
    decoder.unpack(this->gateway_type);
    decoder.unpack(this->algorithm);

    this->gateway.clear();

    switch (static_cast<GatewayType>(this->gateway_type))
    {
        case GatewayType::None:
            decoder.unpack(this->gateway, 1);
            break;

        case GatewayType::IPv4:
            decoder.unpack(this->gateway, 4);
            break;

        case GatewayType::IPv6:
            decoder.unpack(this->gateway, 16);
            break;

        case GatewayType::Domain:
        {
            std::string domain;
            decoder.unpack(domain, true);

            this->gateway.insert(this->gateway.begin(), domain.begin(), domain.end());
        }
            break;
    }

    decoder.unpack(this->publickey, false);
}

void IPSECKEY::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->precedence   = chen::map::find(object, "precedence", this->precedence);
    this->gateway_type = chen::map::find(object, "gateway_type", this->gateway_type);
    this->algorithm    = chen::map::find(object, "algorithm", this->algorithm);

    this->gateway.clear();

    std::string gateway = chen::map::find(object, "gateway", std::string());
    std::copy(gateway.begin(), gateway.end(), this->gateway.begin());

    this->publickey = chen::map::find(object, "publickey", this->publickey);
}


// -----------------------------------------------------------------------------
// RRSIG
RRSIG::RRSIG() : RR(chen::dns::RRType::RRSIG)
{
}

std::string RRSIG::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->type_covered);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + chen::num::str(this->labels);
    ret += " " + chen::num::str(this->original);
    ret += " " + chen::num::str(this->expiration);
    ret += " " + chen::num::str(this->inception);
    ret += " " + chen::num::str(this->key_tag);
    ret += " " + this->signer;
    ret += " " + this->signature;

    return ret;
}

std::shared_ptr<chen::dns::RR> RRSIG::clone() const
{
    return std::make_shared<RRSIG>(*this);
}

void RRSIG::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->type_covered);
    encoder.pack(this->algorithm);
    encoder.pack(this->labels);
    encoder.pack(this->original);
    encoder.pack(this->expiration);
    encoder.pack(this->inception);
    encoder.pack(this->key_tag);
    encoder.pack(this->signer, true);
    encoder.pack(this->signature, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void RRSIG::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->type_covered);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->labels);
    decoder.unpack(this->original);
    decoder.unpack(this->expiration);
    decoder.unpack(this->inception);
    decoder.unpack(this->key_tag);
    decoder.unpack(this->signer, true);
    decoder.unpack(this->signature, false);
}

void RRSIG::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->type_covered = chen::map::find(object, "type_covered", this->type_covered);
    this->algorithm    = chen::map::find(object, "algorithm", this->algorithm);
    this->labels       = chen::map::find(object, "labels", this->labels);
    this->original     = chen::map::find(object, "original", this->original);
    this->expiration   = chen::map::find(object, "expiration", this->expiration);
    this->inception    = chen::map::find(object, "inception", this->inception);
    this->key_tag      = chen::map::find(object, "key_tag", this->key_tag);
    this->signer       = chen::map::find(object, "signer", this->signer);
    this->signature    = chen::map::find(object, "signature", this->signature);
}


// -----------------------------------------------------------------------------
// NSEC
NSEC::NSEC() : RR(chen::dns::RRType::NSEC)
{
}

std::string NSEC::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->next_domain;
    ret += " " + this->escape(this->type_bitmap.size());
    return ret;
}

std::shared_ptr<chen::dns::RR> NSEC::clone() const
{
    return std::make_shared<NSEC>(*this);
}

void NSEC::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->next_domain, true);
    encoder.pack(this->type_bitmap, this->type_bitmap.size());

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NSEC::decode(chen::dns::decoder &decoder)
{
    auto tmp = decoder.cur();

    RR::decode(decoder);
    decoder.unpack(this->next_domain, true);

    this->type_bitmap.clear();
    decoder.unpack(this->type_bitmap, this->remain(tmp, decoder.cur()));
}

void NSEC::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->next_domain = chen::map::find(object, "next_domain", this->next_domain);

    this->type_bitmap.clear();

    std::string type_bitmap = chen::map::find(object, "type_bitmap", std::string());
    std::copy(type_bitmap.begin(), type_bitmap.end(), this->type_bitmap.begin());
}


// -----------------------------------------------------------------------------
// DNSKEY
DNSKEY::DNSKEY() : RR(chen::dns::RRType::DNSKEY)
{
}

std::string DNSKEY::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->flags);
    ret += " " + chen::num::str(this->protocol);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + this->publickey;

    return ret;
}

std::shared_ptr<chen::dns::RR> DNSKEY::clone() const
{
    return std::make_shared<DNSKEY>(*this);
}

void DNSKEY::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->flags);
    encoder.pack(this->protocol);
    encoder.pack(this->algorithm);
    encoder.pack(this->publickey, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void DNSKEY::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->flags);
    decoder.unpack(this->protocol);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->publickey, false);
}

void DNSKEY::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->flags     = chen::map::find(object, "flags", this->flags);
    this->protocol  = chen::map::find(object, "protocol", this->protocol);
    this->algorithm = chen::map::find(object, "algorithm", this->algorithm);
    this->publickey = chen::map::find(object, "publickey", this->publickey);
}


// -----------------------------------------------------------------------------
// DHCID
DHCID::DHCID() : RR(chen::dns::RRType::DHCID)
{
}

std::string DHCID::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->digest;
    return ret;
}

std::shared_ptr<chen::dns::RR> DHCID::clone() const
{
    return std::make_shared<DHCID>(*this);
}

void DHCID::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->digest, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void DHCID::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->digest, false);
}

void DHCID::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->digest = chen::map::find(object, "digest", this->digest);
}


// -----------------------------------------------------------------------------
// NSEC3
NSEC3::NSEC3() : RR(chen::dns::RRType::NSEC3)
{
}

std::string NSEC3::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->hash);
    ret += " " + chen::num::str(this->flags);
    ret += " " + chen::num::str(this->iterations);
    ret += " " + chen::num::str(this->salt_length);
    ret += " " + this->escape(this->salt.size());
    ret += " " + chen::num::str(this->hash_length);
    ret += " " + this->next_owner;
    ret += " " + this->escape(this->type_bitmap.size());

    return ret;
}

std::shared_ptr<chen::dns::RR> NSEC3::clone() const
{
    return std::make_shared<NSEC3>(*this);
}

void NSEC3::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->hash);
    encoder.pack(this->flags);
    encoder.pack(this->iterations);
    encoder.pack(this->salt_length);
    encoder.pack(this->salt, this->salt_length);
    encoder.pack(this->hash_length);
    encoder.pack(this->next_owner, false);
    encoder.pack(this->type_bitmap, this->type_bitmap.size());

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NSEC3::decode(chen::dns::decoder &decoder)
{
    auto tmp = decoder.cur();

    RR::decode(decoder);
    decoder.unpack(this->hash);
    decoder.unpack(this->flags);
    decoder.unpack(this->iterations);
    decoder.unpack(this->salt_length);
    decoder.unpack(this->salt, this->salt_length);
    decoder.unpack(this->hash_length);
    decoder.unpack(this->next_owner, false);

    this->type_bitmap.clear();
    decoder.unpack(this->type_bitmap, this->remain(tmp, decoder.cur()));
}

void NSEC3::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->hash = chen::map::find(object, "hash", this->hash);
    this->flags = chen::map::find(object, "flags", this->flags);
    this->iterations = chen::map::find(object, "iterations", this->iterations);
    this->salt_length = chen::map::find(object, "salt_length", this->salt_length);

    this->salt.clear();

    std::string salt = chen::map::find(object, "salt", std::string());
    std::copy(salt.begin(), salt.end(), this->salt.begin());

    this->hash_length = chen::map::find(object, "hash_length", this->hash_length);
    this->next_owner  = chen::map::find(object, "next_owner", this->next_owner);

    this->type_bitmap.clear();

    std::string type_bitmap = chen::map::find(object, "type_bitmap", std::string());
    std::copy(type_bitmap.begin(), type_bitmap.end(), this->type_bitmap.begin());
}


// -----------------------------------------------------------------------------
// NSEC3PARAM
NSEC3PARAM::NSEC3PARAM() : RR(chen::dns::RRType::NSEC3PARAM)
{
}

std::string NSEC3PARAM::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->hash);
    ret += " " + chen::num::str(this->flags);
    ret += " " + chen::num::str(this->iterations);
    ret += " " + chen::num::str(this->salt_length);
    ret += " " + this->escape(this->salt.size());

    return ret;
}

std::shared_ptr<chen::dns::RR> NSEC3PARAM::clone() const
{
    return std::make_shared<NSEC3PARAM>(*this);
}

void NSEC3PARAM::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->hash);
    encoder.pack(this->flags);
    encoder.pack(this->iterations);
    encoder.pack(this->salt_length);
    encoder.pack(this->salt, this->salt_length);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NSEC3PARAM::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->hash);
    decoder.unpack(this->flags);
    decoder.unpack(this->iterations);
    decoder.unpack(this->salt_length);
    decoder.unpack(this->salt, this->salt_length);
}

void NSEC3PARAM::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->hash        = chen::map::find(object, "hash", this->hash);
    this->flags       = chen::map::find(object, "flags", this->flags);
    this->iterations  = chen::map::find(object, "iterations", this->iterations);
    this->salt_length = chen::map::find(object, "salt_length", this->salt_length);

    this->salt.clear();

    std::string salt = chen::map::find(object, "salt", std::string());
    std::copy(salt.begin(), salt.end(), this->salt.begin());
}


// -----------------------------------------------------------------------------
// TLSA
TLSA::TLSA() : RR(chen::dns::RRType::TLSA)
{
}

std::string TLSA::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->usage);
    ret += " " + chen::num::str(this->selector);
    ret += " " + chen::num::str(this->matching_type);
    ret += " " + this->certificate;

    return ret;
}

std::shared_ptr<chen::dns::RR> TLSA::clone() const
{
    return std::make_shared<TLSA>(*this);
}

void TLSA::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->usage);
    encoder.pack(this->selector);
    encoder.pack(this->matching_type);
    encoder.pack(this->certificate, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void TLSA::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->usage);
    decoder.unpack(this->selector);
    decoder.unpack(this->matching_type);
    decoder.unpack(this->certificate, false);
}

void TLSA::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->usage         = chen::map::find(object, "usage", this->usage);
    this->selector      = chen::map::find(object, "selector", this->selector);
    this->matching_type = chen::map::find(object, "matching_type", this->matching_type);
    this->certificate   = chen::map::find(object, "certificate", this->certificate);
}


// -----------------------------------------------------------------------------
// SMIMEA
SMIMEA::SMIMEA() : RR(chen::dns::RRType::SMIMEA)
{
}

std::string SMIMEA::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->usage);
    ret += " " + chen::num::str(this->selector);
    ret += " " + chen::num::str(this->matching_type);
    ret += " " + this->certificate;

    return ret;
}

std::shared_ptr<chen::dns::RR> SMIMEA::clone() const
{
    return std::make_shared<SMIMEA>(*this);
}

void SMIMEA::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->usage);
    encoder.pack(this->selector);
    encoder.pack(this->matching_type);
    encoder.pack(this->certificate, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void SMIMEA::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->usage);
    decoder.unpack(this->selector);
    decoder.unpack(this->matching_type);
    decoder.unpack(this->certificate, false);
}

void SMIMEA::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->usage         = chen::map::find(object, "usage", this->usage);
    this->selector      = chen::map::find(object, "selector", this->selector);
    this->matching_type = chen::map::find(object, "matching_type", this->matching_type);
    this->certificate   = chen::map::find(object, "certificate", this->certificate);
}


// -----------------------------------------------------------------------------
// HIP
HIP::HIP() : RR(chen::dns::RRType::HIP)
{
}

std::string HIP::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->hit_length);
    ret += " " + chen::num::str(this->pk_algorithm);
    ret += " " + chen::num::str(this->pk_length);
    ret += " " + this->hit;
    ret += " " + this->publickey;
    ret += " " + this->rendezvous_servers;

    return ret;
}

std::shared_ptr<chen::dns::RR> HIP::clone() const
{
    return std::make_shared<HIP>(*this);
}

void HIP::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->hit_length);
    encoder.pack(this->pk_algorithm);
    encoder.pack(this->pk_length);
    encoder.pack(this->hit, false);
    encoder.pack(this->publickey, false);
    encoder.pack(this->rendezvous_servers, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void HIP::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->hit_length);
    decoder.unpack(this->pk_algorithm);
    decoder.unpack(this->pk_length);
    decoder.unpack(this->hit, false);
    decoder.unpack(this->publickey, false);
    decoder.unpack(this->rendezvous_servers, false);
}

void HIP::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->hit_length         = chen::map::find(object, "hit_length", this->hit_length);
    this->pk_algorithm       = chen::map::find(object, "pk_algorithm", this->pk_algorithm);
    this->pk_length          = chen::map::find(object, "pk_length", this->pk_length);
    this->hit                = chen::map::find(object, "hit", this->hit);
    this->publickey          = chen::map::find(object, "publickey", this->publickey);
    this->rendezvous_servers = chen::map::find(object, "rendezvous_servers", this->rendezvous_servers);
}


// -----------------------------------------------------------------------------
// NINFO
NINFO::NINFO() : RR(chen::dns::RRType::NINFO)
{
}

std::string NINFO::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->zs_data;
    return ret;
}

std::shared_ptr<chen::dns::RR> NINFO::clone() const
{
    return std::make_shared<NINFO>(*this);
}

void NINFO::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->zs_data, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NINFO::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->zs_data, false);
}

void NINFO::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->zs_data = chen::map::find(object, "zs_data", this->zs_data);
}


// -----------------------------------------------------------------------------
// RKEY
RKEY::RKEY() : RR(chen::dns::RRType::RKEY)
{
}

std::string RKEY::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->flags);
    ret += " " + chen::num::str(this->protocol);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + this->publickey;

    return ret;
}

std::shared_ptr<chen::dns::RR> RKEY::clone() const
{
    return std::make_shared<RKEY>(*this);
}

void RKEY::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->flags);
    encoder.pack(this->protocol);
    encoder.pack(this->algorithm);
    encoder.pack(this->publickey, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void RKEY::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->flags);
    decoder.unpack(this->protocol);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->publickey, false);
}

void RKEY::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->flags     = chen::map::find(object, "flags", this->flags);
    this->protocol  = chen::map::find(object, "protocol", this->protocol);
    this->algorithm = chen::map::find(object, "algorithm", this->algorithm);
    this->publickey = chen::map::find(object, "publickey", this->publickey);
}


// -----------------------------------------------------------------------------
// TALINK
TALINK::TALINK() : RR(chen::dns::RRType::TALINK)
{
}

std::string TALINK::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + this->previous_name;
    ret += " " + this->next_name;

    return ret;
}

std::shared_ptr<chen::dns::RR> TALINK::clone() const
{
    return std::make_shared<TALINK>(*this);
}

void TALINK::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->previous_name, true);
    encoder.pack(this->next_name, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void TALINK::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->previous_name, true);
    decoder.unpack(this->next_name, true);
}

void TALINK::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->previous_name = chen::map::find(object, "previous_name", this->previous_name);
    this->next_name     = chen::map::find(object, "next_name", this->next_name);
}


// -----------------------------------------------------------------------------
// CDS
CDS::CDS() : RR(chen::dns::RRType::CDS)
{
}

std::string CDS::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->key_tag);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + chen::num::str(this->digest_type);
    ret += " " + this->digest;

    return ret;
}

std::shared_ptr<chen::dns::RR> CDS::clone() const
{
    return std::make_shared<CDS>(*this);
}

void CDS::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->key_tag);
    encoder.pack(this->algorithm);
    encoder.pack(this->digest_type);
    encoder.pack(this->digest, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void CDS::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->key_tag);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->digest_type);
    decoder.unpack(this->digest, false);
}

void CDS::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->key_tag     = chen::map::find(object, "key_tag", this->key_tag);
    this->algorithm   = chen::map::find(object, "algorithm", this->algorithm);
    this->digest_type = chen::map::find(object, "digest_type", this->digest_type);
    this->digest      = chen::map::find(object, "digest", this->digest);
}


// -----------------------------------------------------------------------------
// CDNSKEY
CDNSKEY::CDNSKEY() : RR(chen::dns::RRType::CDNSKEY)
{
}

std::string CDNSKEY::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->flags);
    ret += " " + chen::num::str(this->protocol);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + this->publickey;

    return ret;
}

std::shared_ptr<chen::dns::RR> CDNSKEY::clone() const
{
    return std::make_shared<CDNSKEY>(*this);
}

void CDNSKEY::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->flags);
    encoder.pack(this->protocol);
    encoder.pack(this->algorithm);
    encoder.pack(this->publickey, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void CDNSKEY::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->flags);
    decoder.unpack(this->protocol);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->publickey, false);
}

void CDNSKEY::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->flags     = chen::map::find(object, "flags", this->flags);
    this->protocol  = chen::map::find(object, "protocol", this->protocol);
    this->algorithm = chen::map::find(object, "algorithm", this->algorithm);
    this->publickey = chen::map::find(object, "publickey", this->publickey);
}


// -----------------------------------------------------------------------------
// OPENPGPKEY
OPENPGPKEY::OPENPGPKEY() : RR(chen::dns::RRType::OPENPGPKEY)
{
}

std::string OPENPGPKEY::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->publickey;
    return ret;
}

std::shared_ptr<chen::dns::RR> OPENPGPKEY::clone() const
{
    return std::make_shared<OPENPGPKEY>(*this);
}

void OPENPGPKEY::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->publickey, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void OPENPGPKEY::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->publickey, false);
}

void OPENPGPKEY::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->publickey = chen::map::find(object, "publickey", this->publickey);
}


// -----------------------------------------------------------------------------
// CSYNC
CSYNC::CSYNC() : RR(chen::dns::RRType::CSYNC)
{
}

std::string CSYNC::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->serial);
    ret += " " + chen::num::str(this->flags);
    ret += " " + this->escape(this->type_bitmap.size());

    return ret;
}

std::shared_ptr<chen::dns::RR> CSYNC::clone() const
{
    return std::make_shared<CSYNC>(*this);
}

void CSYNC::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->serial);
    encoder.pack(this->flags);
    encoder.pack(this->type_bitmap, this->type_bitmap.size());

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void CSYNC::decode(chen::dns::decoder &decoder)
{
    auto tmp = decoder.cur();

    RR::decode(decoder);
    decoder.unpack(this->serial);
    decoder.unpack(this->flags);

    this->type_bitmap.clear();
    decoder.unpack(this->type_bitmap, this->remain(tmp, decoder.cur()));
}

void CSYNC::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->serial = chen::map::find(object, "serial", this->serial);
    this->flags  = chen::map::find(object, "flags", this->flags);

    this->type_bitmap.clear();

    std::string type_bitmap = chen::map::find(object, "type_bitmap", std::string());
    std::copy(type_bitmap.begin(), type_bitmap.end(), this->type_bitmap.begin());
}


// -----------------------------------------------------------------------------
// SPF
SPF::SPF() : RR(chen::dns::RRType::SPF)
{
}

std::string SPF::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->escape(this->txt);
    return ret;
}

std::shared_ptr<chen::dns::RR> SPF::clone() const
{
    return std::make_shared<SPF>(*this);
}

void SPF::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->txt, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void SPF::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->txt, false);
}

void SPF::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->txt = chen::map::find(object, "txt", this->txt);
}


// -----------------------------------------------------------------------------
// UINFO
UINFO::UINFO() : Raw(chen::dns::RRType::UINFO)
{
}

std::shared_ptr<chen::dns::RR> UINFO::clone() const
{
    return std::make_shared<UINFO>(*this);
}


// -----------------------------------------------------------------------------
// UID
UID::UID() : Raw(chen::dns::RRType::UID)
{
}

std::shared_ptr<chen::dns::RR> UID::clone() const
{
    return std::make_shared<UID>(*this);
}


// -----------------------------------------------------------------------------
// GID
GID::GID() : Raw(chen::dns::RRType::GID)
{
}

std::shared_ptr<chen::dns::RR> GID::clone() const
{
    return std::make_shared<GID>(*this);
}


// -----------------------------------------------------------------------------
// UNSPEC
UNSPEC::UNSPEC() : Raw(chen::dns::RRType::UNSPEC)
{
}

std::shared_ptr<chen::dns::RR> UNSPEC::clone() const
{
    return std::make_shared<UNSPEC>(*this);
}


// -----------------------------------------------------------------------------
// NID
NID::NID() : RR(chen::dns::RRType::NID)
{
}

std::string NID::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->preference);
    ret += " " + chen::num::str(this->node_id);

    return ret;
}

std::shared_ptr<chen::dns::RR> NID::clone() const
{
    return std::make_shared<NID>(*this);
}

void NID::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->preference);
    encoder.pack(this->node_id);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void NID::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->preference);
    decoder.unpack(this->node_id);
}

void NID::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->preference = chen::map::find(object, "preference", this->preference);
    this->node_id    = chen::map::find(object, "node_id", this->node_id);
}


// -----------------------------------------------------------------------------
// L32
L32::L32() : RR(chen::dns::RRType::L32)
{
}

std::string L32::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->preference);
    ret += " " + chen::num::str(this->locator32);

    return ret;
}

std::shared_ptr<chen::dns::RR> L32::clone() const
{
    return std::make_shared<L32>(*this);
}

void L32::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->preference);
    encoder.pack(this->locator32);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void L32::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->preference);
    decoder.unpack(this->locator32);
}

void L32::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->preference = chen::map::find(object, "preference", this->preference);
    this->locator32  = chen::map::find(object, "locator32", this->locator32);
}


// -----------------------------------------------------------------------------
// L64
L64::L64() : RR(chen::dns::RRType::L64)
{
}

std::string L64::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->preference);
    ret += " " + chen::num::str(this->locator64);

    return ret;
}

std::shared_ptr<chen::dns::RR> L64::clone() const
{
    return std::make_shared<L64>(*this);
}

void L64::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->preference);
    encoder.pack(this->locator64);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void L64::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->preference);
    decoder.unpack(this->locator64);
}

void L64::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->preference = chen::map::find(object, "preference", this->preference);
    this->locator64  = chen::map::find(object, "locator64", this->locator64);
}


// -----------------------------------------------------------------------------
// LP
LP::LP() : RR(chen::dns::RRType::LP)
{
}

std::string LP::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->preference);
    ret += " " + this->fqdn;

    return ret;
}

std::shared_ptr<chen::dns::RR> LP::clone() const
{
    return std::make_shared<LP>(*this);
}

void LP::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->preference);
    encoder.pack(this->fqdn, true);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void LP::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->preference);
    decoder.unpack(this->fqdn, true);
}

void LP::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->preference = chen::map::find(object, "preference", this->preference);
    this->fqdn       = chen::map::find(object, "fqdn", this->fqdn);
}


// -----------------------------------------------------------------------------
// EUI48
EUI48::EUI48() : RR(chen::dns::RRType::EUI48)
{
}

std::string EUI48::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + this->escape(this->address.size());
    return ret;
}

std::shared_ptr<chen::dns::RR> EUI48::clone() const
{
    return std::make_shared<EUI48>(*this);
}

void EUI48::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->address);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void EUI48::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->address);
}

void EUI48::decode(const chen::json::object &object)
{
    RR::decode(object);
    std::string address = chen::map::find(object, "address", std::string());
    std::copy(address.begin(), address.end(), this->address.begin());
}


// -----------------------------------------------------------------------------
// EUI64
EUI64::EUI64() : RR(chen::dns::RRType::EUI64)
{
}

std::string EUI64::str(const std::string &sep) const
{
    auto ret = RR::str(sep);
    ret += sep + chen::num::str(this->address);
    return ret;
}

std::shared_ptr<chen::dns::RR> EUI64::clone() const
{
    return std::make_shared<EUI64>(*this);
}

void EUI64::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->address);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void EUI64::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->address);
}

void EUI64::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->address = chen::map::find(object, "address", this->address);
}


// -----------------------------------------------------------------------------
// TKEY
TKEY::TKEY() : RR(chen::dns::RRType::TKEY)
{
}

std::string TKEY::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + this->algorithm;
    ret += " " + chen::num::str(this->inception);
    ret += " " + chen::num::str(this->expiration);
    ret += " " + chen::num::str(this->mode);
    ret += " " + chen::num::str(this->error);
    ret += " " + chen::num::str(this->key_size);
    ret += " " + this->escape(this->key.size());
    ret += " " + chen::num::str(this->other_len);
    ret += " " + this->escape(this->other_data.size());

    return ret;
}

std::shared_ptr<chen::dns::RR> TKEY::clone() const
{
    return std::make_shared<TKEY>(*this);
}

void TKEY::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->algorithm, true);
    encoder.pack(this->inception);
    encoder.pack(this->expiration);
    encoder.pack(this->mode);
    encoder.pack(this->error);
    encoder.pack(this->key_size);
    encoder.pack(this->key, this->key_size);
    encoder.pack(this->other_len);
    encoder.pack(this->other_data, this->other_len);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void TKEY::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->algorithm, true);
    decoder.unpack(this->inception);
    decoder.unpack(this->expiration);
    decoder.unpack(this->mode);
    decoder.unpack(this->error);
    decoder.unpack(this->key_size);
    decoder.unpack(this->key, this->key_size);
    decoder.unpack(this->other_len);
    decoder.unpack(this->other_data, this->other_len);
}

void TKEY::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->algorithm  = chen::map::find(object, "algorithm", this->algorithm);
    this->inception  = chen::map::find(object, "inception", this->inception);
    this->expiration = chen::map::find(object, "expiration", this->expiration);
    this->mode       = chen::map::find(object, "mode", this->mode);
    this->error      = chen::map::find(object, "error", this->error);
    this->key_size   = chen::map::find(object, "key_size", this->key_size);

    this->key.clear();

    std::string key = chen::map::find(object, "key", std::string());
    std::copy(key.begin(), key.end(), this->key.begin());

    this->other_len = chen::map::find(object, "other_len", this->other_len);

    this->other_data.clear();

    std::string other_data = chen::map::find(object, "other_data", std::string());
    std::copy(other_data.begin(), other_data.end(), this->other_data.begin());
}


// -----------------------------------------------------------------------------
// TSIG
TSIG::TSIG() : RR(chen::dns::RRType::TSIG)
{
}

std::string TSIG::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + this->algorithm;
    ret += " " + this->escape(this->time_signed.size());
    ret += " " + chen::num::str(this->fudge);
    ret += " " + chen::num::str(this->mac_size);
    ret += " " + this->escape(this->mac.size());
    ret += " " + chen::num::str(this->original_id);
    ret += " " + chen::num::str(this->error);
    ret += " " + chen::num::str(this->other_len);
    ret += " " + this->escape(this->other_data.size());

    return ret;
}

std::shared_ptr<chen::dns::RR> TSIG::clone() const
{
    return std::make_shared<TSIG>(*this);
}

void TSIG::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->algorithm, true);
    encoder.pack(this->time_signed);
    encoder.pack(this->fudge);
    encoder.pack(this->mac_size);
    encoder.pack(this->mac, this->mac_size);
    encoder.pack(this->original_id);
    encoder.pack(this->error);
    encoder.pack(this->other_len);
    encoder.pack(this->other_data, this->other_len);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void TSIG::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->algorithm, true);
    decoder.unpack(this->time_signed);
    decoder.unpack(this->fudge);
    decoder.unpack(this->mac_size);
    decoder.unpack(this->mac, this->mac_size);
    decoder.unpack(this->original_id);
    decoder.unpack(this->error);
    decoder.unpack(this->other_len);
    decoder.unpack(this->other_data, this->other_len);
}

void TSIG::decode(const chen::json::object &object)
{
    RR::decode(object);

    this->algorithm = chen::map::find(object, "algorithm", this->algorithm);

    std::string time_signed = chen::map::find(object, "time_signed", std::string());
    std::copy(time_signed.begin(), time_signed.end(), this->time_signed.begin());

    this->fudge    = chen::map::find(object, "fudge", this->fudge);
    this->mac_size = chen::map::find(object, "mac_size", this->mac_size);

    this->mac.clear();

    std::string mac = chen::map::find(object, "mac", std::string());
    std::copy(mac.begin(), mac.end(), this->mac.begin());

    this->original_id = chen::map::find(object, "original_id", this->original_id);
    this->error       = chen::map::find(object, "error", this->error);
    this->other_len   = chen::map::find(object, "other_len", this->other_len);

    this->other_data.clear();

    std::string other_data = chen::map::find(object, "other_data", std::string());
    std::copy(other_data.begin(), other_data.end(), this->other_data.begin());
}


// -----------------------------------------------------------------------------
// URI
URI::URI() : RR(chen::dns::RRType::URI)
{
}

std::string URI::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->priority);
    ret += " " + chen::num::str(this->weight);
    ret += " " + this->target;

    return ret;
}

std::shared_ptr<chen::dns::RR> URI::clone() const
{
    return std::make_shared<URI>(*this);
}

void URI::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->priority);
    encoder.pack(this->weight);
    encoder.pack(this->target, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void URI::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->priority);
    decoder.unpack(this->weight);
    decoder.unpack(this->target, false);
}

void URI::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->priority = chen::map::find(object, "priority", this->priority);
    this->weight   = chen::map::find(object, "weight", this->weight);
    this->target   = chen::map::find(object, "target", this->target);
}


// -----------------------------------------------------------------------------
// CAA
CAA::CAA() : RR(chen::dns::RRType::CAA)
{
}

std::string CAA::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->flags);
    ret += " " + this->tag;
    ret += " " + this->value;

    return ret;
}

std::shared_ptr<chen::dns::RR> CAA::clone() const
{
    return std::make_shared<CAA>(*this);
}

void CAA::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->flags);
    encoder.pack(this->tag, false);
    encoder.pack(this->value, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void CAA::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->flags);
    decoder.unpack(this->tag, false);
    decoder.unpack(this->value, false);
}

void CAA::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->flags = chen::map::find(object, "flags", this->flags);
    this->tag   = chen::map::find(object, "tag", this->tag);
    this->value = chen::map::find(object, "value", this->value);
}


// -----------------------------------------------------------------------------
// TA
TA::TA() : RR(chen::dns::RRType::TA)
{
}

std::string TA::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->key_tag);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + chen::num::str(this->digest_type);
    ret += " " + this->digest;

    return ret;
}

std::shared_ptr<chen::dns::RR> TA::clone() const
{
    return std::make_shared<TA>(*this);
}

void TA::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->key_tag);
    encoder.pack(this->algorithm);
    encoder.pack(this->digest_type);
    encoder.pack(this->digest, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void TA::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->key_tag);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->digest_type);
    decoder.unpack(this->digest, false);
}

void TA::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->key_tag     = chen::map::find(object, "key_tag", this->key_tag);
    this->algorithm   = chen::map::find(object, "algorithm", this->algorithm);
    this->digest_type = chen::map::find(object, "digest_type", this->digest_type);
    this->digest      = chen::map::find(object, "digest", this->digest);
}


// -----------------------------------------------------------------------------
// DLV
DLV::DLV() : RR(chen::dns::RRType::DLV)
{
}

std::string DLV::str(const std::string &sep) const
{
    auto ret = RR::str(sep);

    ret += sep + chen::num::str(this->key_tag);
    ret += " " + chen::num::str(this->algorithm);
    ret += " " + chen::num::str(this->digest_type);
    ret += " " + this->digest;

    return ret;
}

std::shared_ptr<chen::dns::RR> DLV::clone() const
{
    return std::make_shared<DLV>(*this);
}

void DLV::encode(chen::dns::encoder &encoder) const
{
    // base
    RR::encode(encoder);

    // self
    auto val = encoder.size();

    encoder.pack(this->key_tag);
    encoder.pack(this->algorithm);
    encoder.pack(this->digest_type);
    encoder.pack(this->digest, false);

    // rdlength
    this->adjust(encoder, encoder.size() - val);
}

void DLV::decode(chen::dns::decoder &decoder)
{
    RR::decode(decoder);
    decoder.unpack(this->key_tag);
    decoder.unpack(this->algorithm);
    decoder.unpack(this->digest_type);
    decoder.unpack(this->digest, false);
}

void DLV::decode(const chen::json::object &object)
{
    RR::decode(object);
    this->key_tag     = chen::map::find(object, "key_tag", this->key_tag);
    this->algorithm   = chen::map::find(object, "algorithm", this->algorithm);
    this->digest_type = chen::map::find(object, "digest_type", this->digest_type);
    this->digest      = chen::map::find(object, "digest", this->digest);
}