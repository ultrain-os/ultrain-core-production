#include <ultrainio/chain/worldstate.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <fc/scoped_exit.hpp>

namespace ultrainio { namespace chain {

variant_worldstate_writer::variant_worldstate_writer(fc::mutable_variant_object& worldstate)
: worldstate(worldstate)
{
   worldstate.set("sections", fc::variants());
   worldstate.set("version", current_worldstate_version );
}

void variant_worldstate_writer::write_start_section( const std::string& section_name ) {
   current_rows.clear();
   current_section_name = section_name;
}

void variant_worldstate_writer::write_row( const detail::abstract_worldstate_row_writer& row_writer ) {
   current_rows.emplace_back(row_writer.to_variant());
}

void variant_worldstate_writer::write_end_section( ) {
   worldstate["sections"].get_array().emplace_back(fc::mutable_variant_object()("name", std::move(current_section_name))("rows", std::move(current_rows)));
}

void variant_worldstate_writer::finalize() {

}

variant_worldstate_reader::variant_worldstate_reader(const fc::variant& worldstate)
:worldstate(worldstate)
,cur_row(0)
{
}

void variant_worldstate_reader::validate() const {
   ULTRAIN_ASSERT(worldstate.is_object(), worldstate_validation_exception,
         "Variant worldstate is not an object");
   const fc::variant_object& o = worldstate.get_object();

   ULTRAIN_ASSERT(o.contains("version"), worldstate_validation_exception,
         "Variant worldstate has no version");

   const auto& version = o["version"];
   ULTRAIN_ASSERT(version.is_integer(), worldstate_validation_exception,
         "Variant worldstate version is not an integer");

   ULTRAIN_ASSERT(version.as_uint64() == (uint64_t)current_worldstate_version, worldstate_validation_exception,
         "Variant worldstate is an unsuppored version.  Expected : ${expected}, Got: ${actual}",
         ("expected", current_worldstate_version)("actual",o["version"].as_uint64()));

   ULTRAIN_ASSERT(o.contains("sections"), worldstate_validation_exception,
         "Variant worldstate has no sections");

   const auto& sections = o["sections"];
   ULTRAIN_ASSERT(sections.is_array(), worldstate_validation_exception, "Variant worldstate sections is not an array");

   const auto& section_array = sections.get_array();
   for( const auto& section: section_array ) {
      ULTRAIN_ASSERT(section.is_object(), worldstate_validation_exception, "Variant worldstate section is not an object");

      const auto& so = section.get_object();
      ULTRAIN_ASSERT(so.contains("name"), worldstate_validation_exception,
            "Variant worldstate section has no name");

      ULTRAIN_ASSERT(so["name"].is_string(), worldstate_validation_exception,
                 "Variant worldstate section name is not a string");

      ULTRAIN_ASSERT(so.contains("rows"), worldstate_validation_exception,
                 "Variant worldstate section has no rows");

      ULTRAIN_ASSERT(so["rows"].is_array(), worldstate_validation_exception,
                 "Variant worldstate section rows is not an array");
   }
}

bool variant_worldstate_reader::has_section( const string& section_name ) {
   const auto& sections = worldstate["sections"].get_array();
   for( const auto& section: sections ) {
      if (section["name"].as_string() == section_name) {
         return true;
      }
   }

   return false;
}

void variant_worldstate_reader::set_section( const string& section_name ) {
   const auto& sections = worldstate["sections"].get_array();
   for( const auto& section: sections ) {
      if (section["name"].as_string() == section_name) {
         cur_section = &section.get_object();
         return;
      }
   }

   ULTRAIN_THROW(worldstate_exception, "Variant worldstate has no section named ${n}", ("n", section_name));
}

bool variant_worldstate_reader::read_row( detail::abstract_worldstate_row_reader& row_reader ) {
   const auto& rows = (*cur_section)["rows"].get_array();
   row_reader.provide(rows.at(cur_row++));
   return cur_row < rows.size();
}

bool variant_worldstate_reader::empty ( ) {
   const auto& rows = (*cur_section)["rows"].get_array();
   return rows.empty();
}

void variant_worldstate_reader::clear_section() {
   cur_section = nullptr;
   cur_row = 0;
}

ostream_worldstate_writer::ostream_worldstate_writer(std::ostream& worldstate)
:worldstate(worldstate)
,header_pos(worldstate.tellp())
,section_pos(-1)
,row_count(0)
{
   // write magic number
   auto totem = magic_number;
   worldstate.write((char*)&totem, sizeof(totem));

   // write version
   auto version = current_worldstate_version;
   worldstate.write((char*)&version, sizeof(version));
}

void ostream_worldstate_writer::write_start_section( const std::string& section_name )
{
   ULTRAIN_ASSERT(section_pos == std::streampos(-1), worldstate_exception, "Attempting to write a new section without closing the previous section");
   section_pos = worldstate.tellp();
   row_count = 0;

   uint64_t placeholder = std::numeric_limits<uint64_t>::max();

   // write a placeholder for the section size
   worldstate.write((char*)&placeholder, sizeof(placeholder));

   // write placeholder for row count
   worldstate.write((char*)&placeholder, sizeof(placeholder));

   // write the section name (null terminated)
   worldstate.write(section_name.data(), section_name.size());
   worldstate.put(0);
}

void ostream_worldstate_writer::write_row( const detail::abstract_worldstate_row_writer& row_writer ) {
   auto restore = worldstate.tellp();
   try {
      row_writer.write(worldstate);
   } catch (...) {
      worldstate.seekp(restore);
      throw;
   }
   row_count++;
}

void ostream_worldstate_writer::write_end_section( ) {
   auto restore = worldstate.tellp();

   uint64_t section_size = restore - section_pos - sizeof(uint64_t);

   worldstate.seekp(section_pos);

   // write a the section size
   worldstate.write((char*)&section_size, sizeof(section_size));

   // write the row count
   worldstate.write((char*)&row_count, sizeof(row_count));

   worldstate.seekp(restore);

   section_pos = std::streampos(-1);
   row_count = 0;
}

void ostream_worldstate_writer::finalize() {
   uint64_t end_marker = std::numeric_limits<uint64_t>::max();

   // write a placeholder for the section size
   worldstate.write((char*)&end_marker, sizeof(end_marker));
}

istream_worldstate_reader::istream_worldstate_reader(std::istream& worldstate)
:worldstate(worldstate)
,header_pos(worldstate.tellg())
,num_rows(0)
,cur_row(0)
{

}

void istream_worldstate_reader::validate() const {
   // make sure to restore the read pos
   auto restore_pos = fc::make_scoped_exit([this,pos=worldstate.tellg(),ex=worldstate.exceptions()](){
      worldstate.seekg(pos);
      worldstate.exceptions(ex);
   });

   worldstate.exceptions(std::istream::failbit|std::istream::eofbit);

   try {
      // validate totem
      auto expected_totem = ostream_worldstate_writer::magic_number;
      decltype(expected_totem) actual_totem;
      worldstate.read((char*)&actual_totem, sizeof(actual_totem));
      ULTRAIN_ASSERT(actual_totem == expected_totem, worldstate_exception,
                 "Binary worldstate has unexpected magic number!");

      // validate version
      auto expected_version = current_worldstate_version;
      decltype(expected_version) actual_version;
      worldstate.read((char*)&actual_version, sizeof(actual_version));
      ULTRAIN_ASSERT(actual_version == expected_version, worldstate_exception,
                 "Binary worldstate is an unsuppored version.  Expected : ${expected}, Got: ${actual}",
                 ("expected", expected_version)("actual", actual_version));

      while (validate_section()) {}
   } catch( const std::exception& e ) {  \
      worldstate_exception fce(FC_LOG_MESSAGE( warn, "Binary worldstate validation threw IO exception (${what})",("what",e.what())));
      throw fce;
   }
}

bool istream_worldstate_reader::validate_section() const {
   uint64_t section_size = 0;
   worldstate.read((char*)&section_size,sizeof(section_size));

   // stop when we see the end marker
   if (section_size == std::numeric_limits<uint64_t>::max()) {
      return false;
   }

   // seek past the section
   worldstate.seekg(worldstate.tellg() + std::streamoff(section_size));

   return true;
}

bool istream_worldstate_reader::has_section( const string& section_name ) {
   auto restore_pos = fc::make_scoped_exit([this,pos=worldstate.tellg()](){
      worldstate.seekg(pos);
   });

   const std::streamoff header_size = sizeof(ostream_worldstate_writer::magic_number) + sizeof(current_worldstate_version);

   auto next_section_pos = header_pos + header_size;

   while (true) {
      worldstate.seekg(next_section_pos);
      uint64_t section_size = 0;
      worldstate.read((char*)&section_size,sizeof(section_size));
      if (section_size == std::numeric_limits<uint64_t>::max()) {
         break;
      }

      next_section_pos = worldstate.tellg() + std::streamoff(section_size);

      uint64_t ignore = 0;
      worldstate.read((char*)&ignore,sizeof(ignore));

      bool match = true;
      for(auto c : section_name) {
         if(worldstate.get() != c) {
            match = false;
            break;
         }
      }

      if (match && worldstate.get() == 0) {
         return true;
      }
   }

   return false;
}

void istream_worldstate_reader::set_section( const string& section_name ) {
   auto restore_pos = fc::make_scoped_exit([this,pos=worldstate.tellg()](){
      worldstate.seekg(pos);
   });

   const std::streamoff header_size = sizeof(ostream_worldstate_writer::magic_number) + sizeof(current_worldstate_version);

   auto next_section_pos = header_pos + header_size;

   while (true) {
      worldstate.seekg(next_section_pos);
      uint64_t section_size = 0;
      worldstate.read((char*)&section_size,sizeof(section_size));
      if (section_size == std::numeric_limits<uint64_t>::max()) {
         break;
      }

      next_section_pos = worldstate.tellg() + std::streamoff(section_size);

      uint64_t row_count = 0;
      worldstate.read((char*)&row_count,sizeof(row_count));

      bool match = true;
      for(auto c : section_name) {
         if(worldstate.get() != c) {
            match = false;
            break;
         }
      }

      if (match && worldstate.get() == 0) {
         cur_row = 0;
         num_rows = row_count;

         // leave the stream at the right point
         restore_pos.cancel();
         return;
      }
   }

   ULTRAIN_THROW(worldstate_exception, "Binary worldstate has no section named ${n}", ("n", section_name));
}

bool istream_worldstate_reader::read_row( detail::abstract_worldstate_row_reader& row_reader ) {
   row_reader.provide(worldstate);
   return ++cur_row < num_rows;
}

bool istream_worldstate_reader::empty ( ) {
   return num_rows == 0;
}

void istream_worldstate_reader::clear_section() {
   num_rows = 0;
   cur_row = 0;
}

integrity_hash_worldstate_writer::integrity_hash_worldstate_writer(fc::sha256::encoder& enc)
:enc(enc)
{
}

void integrity_hash_worldstate_writer::write_start_section( const std::string& )
{
   // no-op for structural details
}

void integrity_hash_worldstate_writer::write_row( const detail::abstract_worldstate_row_writer& row_writer ) {
   row_writer.write(enc);
}

void integrity_hash_worldstate_writer::write_end_section( ) {
   // no-op for structural details
}

void integrity_hash_worldstate_writer::finalize() {
   // no-op for structural details
}

}}
