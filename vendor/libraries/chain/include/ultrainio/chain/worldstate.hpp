/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainio/chain/database_utils.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <fc/variant_object.hpp>
#include <boost/core/demangle.hpp>
#include <ostream>

namespace ultrainio { namespace chain {
   /**
    * History:
    * Version 1: initial version with string identified sections and rows
    */
   static const uint32_t current_worldstate_version = 1;

   namespace detail {
      template<typename T>
      struct worldstate_section_traits {
         static std::string section_name() {
            return boost::core::demangle(typeid(T).name());
         }
      };

      template<typename T>
      struct worldstate_row_traits {
         using value_type = std::decay_t<T>;
         using worldstate_type = value_type;

         static const worldstate_type& to_worldstate_row( const value_type& value, const chainbase::database&, void* data = nullptr) {
            return value;
         };
      };

      /**
       * Due to a pattern in our code of overloading `operator << ( std::ostream&, ... )` to provide
       * human-readable string forms of data, we cannot directly use ostream as those operators will
       * be used instead of the expected operators.  In otherwords:
       * fc::raw::pack(fc::datastream...)
       * will end up calling _very_ different operators than
       * fc::raw::pack(std::ostream...)
       */
      struct ostream_wrapper {
         explicit ostream_wrapper(std::ostream& s)
         :inner(s) {

         }

         ostream_wrapper(ostream_wrapper &&) = default;
         ostream_wrapper(const ostream_wrapper& ) = default;

         auto& write( const char* d, size_t s ) {
            return inner.write(d, s);
         }

         auto& put(char c) {
           return inner.put(c);
         }

         auto tellp() const {
            return inner.tellp();
         }

         auto& seekp(std::ostream::pos_type p) {
            return inner.seekp(p);
         }

         std::ostream& inner;
      };


      struct abstract_worldstate_row_writer {
         virtual void write(ostream_wrapper& out) const = 0;
         virtual void write(fc::sha256::encoder& out) const = 0;
         virtual variant to_variant() const = 0;
         virtual std::string row_type_name() const = 0;
      };

      template<typename T>
      struct worldstate_row_writer : abstract_worldstate_row_writer {
         explicit worldstate_row_writer( const T& data )
         :data(data) {}

         template<typename DataStream>
         void write_stream(DataStream& out) const {
            fc::raw::pack(out, data);
         }

         void write(ostream_wrapper& out) const override {
            write_stream(out);
         }

         void write(fc::sha256::encoder& out) const override {
            write_stream(out);
         }

         fc::variant to_variant() const override {
            variant var;
            fc::to_variant(data, var);
            return var;
         }

         std::string row_type_name() const override {
            return boost::core::demangle( typeid( T ).name() );
         }

         const T& data;
      };

      template<typename T>
      worldstate_row_writer<T> make_row_writer( const T& data) {
         return worldstate_row_writer<T>(data);
      }
   }

   class worldstate_writer {
      public:
         class section_writer {
            public:
               template<typename T>
               void add_row( const T& row, const chainbase::database& db, void* data = nullptr ) {
                  _writer.write_row(detail::make_row_writer(detail::worldstate_row_traits<T>::to_worldstate_row(row, db, data)));
               }

               void add_row( std::vector<char>& in_data ) {
                  _writer.write_row(in_data);
               }

            private:
               friend class worldstate_writer;
               section_writer(worldstate_writer& writer)
               :_writer(writer)
               {

               }
               worldstate_writer& _writer;
         };

         template<typename F>
         void write_section(const std::string section_name, F f) {
            write_start_section(section_name);
            auto section = section_writer(*this);
            f(section);
            write_end_section();
         }

         template<typename T, typename F>
         void write_section(F f) {
            write_section(detail::worldstate_section_traits<T>::section_name(), f);
         }

      virtual ~worldstate_writer(){};

      protected:
         virtual void write_start_section( const std::string& section_name ) = 0;
         virtual void write_row( const detail::abstract_worldstate_row_writer& row_writer ) = 0;
         virtual void write_row( std::vector<char>& in_data ) {};
         virtual void write_end_section() = 0;
   };

   using worldstate_writer_ptr = std::shared_ptr<worldstate_writer>;

   namespace detail {
      struct abstract_worldstate_row_reader {
         virtual void provide(std::istream& in) const = 0;
         virtual void provide(const fc::variant&) const = 0;
         virtual std::string row_type_name() const = 0;
      };

      template<typename T>
      struct is_chainbase_object {
         static constexpr bool value = false;
      };

      template<uint16_t TypeNumber, typename Derived>
      struct is_chainbase_object<chainbase::object<TypeNumber, Derived>> {
         static constexpr bool value = true;
      };

      template<typename T>
      constexpr bool is_chainbase_object_v = is_chainbase_object<T>::value;

      struct row_validation_helper {
         template<typename T, typename F>
         static auto apply(const T& data, F f) -> std::enable_if_t<is_chainbase_object_v<T>> {
            auto orig = data.id;
            f();
            ULTRAIN_ASSERT(orig == data.id, worldstate_exception,
                       "Snapshot for ${type} mutates row member \"id\" which is illegal",
                       ("type",boost::core::demangle( typeid( T ).name() )));
         }

         template<typename T, typename F>
         static auto apply(const T&, F f) -> std::enable_if_t<!is_chainbase_object_v<T>> {
            f();
         }
      };

      template<typename T>
      struct worldstate_row_reader : abstract_worldstate_row_reader {
         explicit worldstate_row_reader( T& data )
         :data(data) {}


         void provide(std::istream& in) const override {
            row_validation_helper::apply(data, [&in,this](){
               fc::raw::unpack(in, data);
            });
         }

         void provide(const fc::variant& var) const override {
            row_validation_helper::apply(data, [&var,this]() {
               fc::from_variant(var, data);
            });
         }

         std::string row_type_name() const override {
            return boost::core::demangle( typeid( T ).name() );
         }

         T& data;
      };

      template<typename T>
      worldstate_row_reader<T> make_row_reader( T& data ) {
         return worldstate_row_reader<T>(data);
      }
   }

   class worldstate_reader {
      public:
         class section_reader {
            public:
               template<typename T>
               auto read_row( T& out ) -> std::enable_if_t<std::is_same<std::decay_t<T>, typename detail::worldstate_row_traits<T>::worldstate_type>::value,bool> {
                  auto reader = detail::make_row_reader(out);
                  return _reader.read_row(reader);
               }

               template<typename T>
               auto read_row( T& out, chainbase::database&, bool backup = false, void* data = nullptr) -> std::enable_if_t<std::is_same<std::decay_t<T>, typename detail::worldstate_row_traits<T>::worldstate_type>::value,bool> {
                  return read_row(out);
               }

               template<typename T>
               auto read_row( T& out, chainbase::database& db, bool backup, void* data) -> std::enable_if_t<!std::is_same<std::decay_t<T>, typename detail::worldstate_row_traits<T>::worldstate_type>::value,bool> {
                  auto temp = typename detail::worldstate_row_traits<T>::worldstate_type();
                  auto reader = detail::make_row_reader(temp);
                  bool result = _reader.read_row(reader);
                  detail::worldstate_row_traits<T>::from_worldstate_row(std::move(temp), out, db, backup, data);
                  return result;
               }

               bool empty() {
                  return _reader.empty();
               }

            private:
               friend class worldstate_reader;
               section_reader(worldstate_reader& _reader)
               :_reader(_reader)
               {}

               worldstate_reader& _reader;

         };

      template<typename F>
      void read_section(const std::string& section_name, F f) {
         set_section(section_name);
         auto section = section_reader(*this);
         f(section);
         clear_section();
      }

      template<typename T, typename F>
      void read_section(F f) {
         read_section(detail::worldstate_section_traits<T>::section_name(), f);
      }

      template<typename T>
      bool has_section(const std::string& suffix = std::string()) {
         return has_section(suffix + detail::worldstate_section_traits<T>::section_name());
      }

      template<typename T>
      bool get_section_info(uint64_t& section_size, uint64_t& row_count, int& data_pos, const std::string& suffix = std::string()) {
         return get_section_info(section_size, row_count, data_pos, suffix + detail::worldstate_section_traits<T>::section_name());
      }

      virtual void validate() const = 0;

      virtual ~worldstate_reader(){};

      protected:
         virtual bool has_section( const std::string& section_name ) = 0;
         virtual bool get_section_info(uint64_t& section_size, uint64_t& row_count, int& data_pos, const std::string& section_name){ return false; };
         virtual void set_section( const std::string& section_name ) = 0;
         virtual bool read_row( detail::abstract_worldstate_row_reader& row_reader ) = 0;
         virtual bool empty( ) = 0;
         virtual void clear_section() = 0;
   };

   using worldstate_reader_ptr = std::shared_ptr<worldstate_reader>;

   class variant_worldstate_writer : public worldstate_writer {
      public:
         variant_worldstate_writer(fc::mutable_variant_object& worldstate);

         void write_start_section( const std::string& section_name ) override;
         void write_row( const detail::abstract_worldstate_row_writer& row_writer ) override;
         void write_end_section( ) override;
         void finalize();

      private:
         fc::mutable_variant_object& worldstate;
         std::string current_section_name;
         fc::variants current_rows;
   };

   class variant_worldstate_reader : public worldstate_reader {
      public:
         explicit variant_worldstate_reader(const fc::variant& worldstate);

         void validate() const override;
         bool has_section( const string& section_name ) override;
         void set_section( const string& section_name ) override;
         bool read_row( detail::abstract_worldstate_row_reader& row_reader ) override;
         bool empty ( ) override;
         void clear_section() override;

      private:
         const fc::variant& worldstate;
         const fc::variant_object* cur_section;
         uint64_t cur_row;
   };

   class ostream_worldstate_writer : public worldstate_writer {
      public:
         explicit ostream_worldstate_writer(std::ostream& worldstate);

         void write_start_section( const std::string& section_name ) override;
         void write_row( const detail::abstract_worldstate_row_writer& row_writer ) override;
         void write_row( std::vector<char>& in_data ) override;
         void write_end_section( ) override;
         void finalize();
         uint64_t write_length();

         static const uint32_t magic_number = 0x30510550;

      private:
         detail::ostream_wrapper worldstate;
         std::streampos          header_pos;
         std::streampos          section_pos;
         uint64_t                row_count;
         uint64_t	               length_write;

   };

   class istream_worldstate_reader : public worldstate_reader {
      public:
         explicit istream_worldstate_reader(std::istream& worldstate);

         void validate() const override;
         bool has_section( const string& section_name ) override;
         bool get_section_info(uint64_t& section_size, uint64_t& row_count, int& data_pos, const std::string& section_name) override;
         void set_section( const string& section_name ) override;
         bool read_row( detail::abstract_worldstate_row_reader& row_reader ) override;
         bool empty ( ) override;
         void clear_section() override;
         uint64_t read_length();
         bool read_row(uint64_t size, std::vector<char>& out_data);

      private:
         bool validate_section() const;

         std::istream&  worldstate;
         std::streampos header_pos;
         uint64_t       num_rows;
         uint64_t       cur_row;
         uint64_t	      length_read;
   };

   class integrity_hash_worldstate_writer : public worldstate_writer {
      public:
         explicit integrity_hash_worldstate_writer(fc::sha256::encoder&  enc);

         void write_start_section( const std::string& section_name ) override;
         void write_row( const detail::abstract_worldstate_row_writer& row_writer ) override;
         void write_end_section( ) override;
         void finalize();

      private:
         fc::sha256::encoder&  enc;

   };

   class ostream_worldstate_id_writer {
      public:
         explicit ostream_worldstate_id_writer(std::ostream& worldstate_id_ostream);
         void write_start_id_section( const std::string& section_name );
         void write_row_id( const uint64_t id, const uint64_t size);
         void write_end_id_section( );
         void finalize();

         static const uint32_t magic_number = 0x30510550;

      private:
         detail::ostream_wrapper worldstate_id_ostream;
         std::streampos          header_pos;
         std::streampos          section_pos;
         uint64_t                row_count;

   };

   class istream_worldstate_id_reader {
      public:
         explicit istream_worldstate_id_reader(std::istream& worldstate_id_ostream);

         void validate() const;
         bool has_id_section( const string& section_name );
         void read_start_id_section( const string& section_name );
         bool read_id_row(uint64_t& id, uint64_t& size);
         bool empty ( );
         void clear_id_section();
         bool is_more();

      private:
         bool validate_id_section() const;

         std::istream&  worldstate_id_ostream;
         std::streampos header_pos;
         uint64_t       num_rows;
         uint64_t       cur_row;
   };

}}
