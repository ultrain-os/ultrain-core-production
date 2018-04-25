/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainio/utilities/tempdir.hpp>

#include <cstdlib>

namespace ultrainio { namespace utilities {

fc::path temp_directory_path()
{
   const char* ultrain_tempdir = getenv("ULTRAIN_TEMPDIR");
   if( ultrain_tempdir != nullptr )
      return fc::path( ultrain_tempdir );
   return fc::temp_directory_path() / "ultrain-tmp";
}

} } // ultrainio::utilities
