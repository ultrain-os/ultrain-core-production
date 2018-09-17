//
// sync_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <regex>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
//#include <boost/asio/ssl.hpp>
#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <ultrainio/chain/exceptions.hpp>
//#include <ultrainio/http_plugin/http_plugin.hpp>
//#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include "httpc_only_send.hpp"

using boost::asio::ip::tcp;
using namespace ultrainio::chain;
namespace ultrainio { namespace client { namespace http {

   namespace detail {
      class http_context_impl {
         public:
            boost::asio::io_service ios;
      };

      void http_context_deleter::operator()(http_context_impl* p) const {
         delete p;
      }
   }

   http_context create_http_context() {
      return http_context(new detail::http_context_impl, detail::http_context_deleter());
   }

   void do_connect(tcp::socket& sock, const resolved_url& url) {
      // Get a list of endpoints corresponding to the server name.
      vector<tcp::endpoint> endpoints;
      endpoints.reserve(url.resolved_addresses.size());
      for (const auto& addr: url.resolved_addresses) {
         endpoints.emplace_back(boost::asio::ip::make_address(addr), url.resolved_port);
      }
      boost::asio::connect(sock, endpoints);
   }

   template<class T>
   std::string do_txrx(T& socket, boost::asio::streambuf& request_buff, unsigned int& status_code) {
      // Send the request.
      boost::asio::write(socket, request_buff);

      // Read the response status line. The response streambuf will automatically
      // grow to accommodate the entire line. The growth may be limited by passing
      // a maximum size to the streambuf constructor.
      boost::asio::streambuf response;
      boost::asio::read_until(socket, response, "\r\n");

      // Check that response is OK.
      std::istream response_stream(&response);
      std::string http_version;
      response_stream >> http_version;
      response_stream >> status_code;

      ULTRAIN_ASSERT( status_code != 400, invalid_http_request, "The server has rejected the request as invalid!");

      std::string status_message;
      std::getline(response_stream, status_message);
      ULTRAIN_ASSERT( !(!response_stream || http_version.substr(0, 5) != "HTTP/"), invalid_http_response, "Invalid Response" );

      // Read the response headers, which are terminated by a blank line.
      boost::asio::read_until(socket, response, "\r\n\r\n");

      // Process the response headers.
      std::string header;
      int response_content_length = -1;
      std::regex clregex(R"xx(^content-length:\s+(\d+))xx", std::regex_constants::icase);
      while (std::getline(response_stream, header) && header != "\r") {
         std::smatch match;
         if(std::regex_search(header, match, clregex))
            response_content_length = std::stoi(match[1]);
      }
      ULTRAIN_ASSERT(response_content_length >= 0, invalid_http_response, "Invalid content-length response");

      std::stringstream re;
      // Write whatever content we already have to output.
      response_content_length -= response.size();
      if (response.size() > 0)
         re << &response;

      boost::asio::read(socket, response, boost::asio::transfer_exactly(response_content_length));
      re << &response;

      return re.str();
   }

   parsed_url parse_url( const string& server_url ) {
      parsed_url res;

      //via rfc3986 and modified a bit to suck out the port number
      //Sadly this doesn't work for ipv6 addresses
      std::regex rgx(R"xx(^(([^:/?#]+):)?(//([^:/?#]*)(:(\d+))?)?([^?#]*)(\?([^#]*))?(#(.*))?)xx");
      std::smatch match;
      if(std::regex_search(server_url.begin(), server_url.end(), match, rgx)) {
         res.scheme = match[2];
         res.server = match[4];
         res.port = match[6];
         res.path = match[7];
      }
      if(res.scheme != "http" && res.scheme != "https")
         ULTRAIN_THROW(fail_to_resolve_host, "Unrecognized URL scheme (${s}) in URL \"${u}\"", ("s", res.scheme)("u", server_url));
      if(res.server.empty())
         ULTRAIN_THROW(fail_to_resolve_host, "No server parsed from URL \"${u}\"", ("u", server_url));
      if(res.port.empty())
         res.port = res.scheme == "http" ? "80" : "443";
      boost::trim_right_if(res.path, boost::is_any_of("/"));
      return res;
   }

   resolved_url resolve_url( const http_context& context, const parsed_url& url ) {
      tcp::resolver resolver(context->ios);
      boost::system::error_code ec;
      auto result = resolver.resolve(tcp::v4(), url.server, url.port, ec);
      if (ec) {
         ULTRAIN_THROW(fail_to_resolve_host, "Error resolving \"${server}:${url}\" : ${m}", ("server", url.server)("port",url.port)("m",ec.message()));
      }

      // non error results are guaranteed to return a non-empty range
      vector<string> resolved_addresses;
      resolved_addresses.reserve(result.size());
      fc::optional<uint16_t> resolved_port;
      bool is_loopback = true;

      for(const auto& r : result) {
         const auto& addr = r.endpoint().address();
         if (addr.is_v6()) continue;
         uint16_t port = r.endpoint().port();
         resolved_addresses.emplace_back(addr.to_string());
         is_loopback = is_loopback && addr.is_loopback();

         if (resolved_port) {
            ULTRAIN_ASSERT(*resolved_port == port, resolved_to_multiple_ports, "Service name \"${port}\" resolved to multiple ports and this is not supported!", ("port",url.port));
         } else {
            resolved_port = port;
         }
      }

      return resolved_url(url, std::move(resolved_addresses), *resolved_port, is_loopback);
   }

   string format_host_header(const resolved_url& url) {
      // common practice is to only make the port explicit when it is the non-default port
      if (
         (url.scheme == "https" && url.resolved_port == 443) ||
         (url.scheme == "http" && url.resolved_port == 80)
      ) {
         return url.server;
      } else {
         return url.server + ":" + url.port;
      }
   }

   fc::variant do_http_call( const connection_param& cp,
                             const fc::variant& postdata,
                             bool print_request,
                             bool print_response ) {
   std::string postjson;
   if( !postdata.is_null() ) {
      postjson = print_request ? fc::json::to_pretty_string( postdata ) : fc::json::to_string( postdata );
   }

   const auto& url = cp.url;

   boost::asio::streambuf request;
   std::ostream request_stream(&request);
   auto host_header_value = format_host_header(url);
   request_stream << "POST " << url.path << " HTTP/1.0\r\n";
   request_stream << "Host: " << host_header_value << "\r\n";
   request_stream << "content-length: " << postjson.size() << "\r\n";
   request_stream << "Accept: */*\r\n";
   request_stream << "Connection: close\r\n";
   request_stream << "\r\n";
   // append more customized headers
   std::vector<string>::iterator itr;
   for (itr = cp.headers.begin(); itr != cp.headers.end(); itr++) {
      request_stream << *itr << "\r\n";
   }
   request_stream << postjson;

   if ( print_request ) {
      string s(request.size(), '\0');
      buffer_copy(boost::asio::buffer(s), request.data());
      std::cerr << "REQUEST:" << std::endl
                << "---------------------" << std::endl
                << s << std::endl
                << "---------------------" << std::endl;
   }

   std::string re;

   try {
      if(url.scheme == "http") {
         tcp::socket socket(cp.context->ios);
         do_connect(socket, url);
         boost::asio::write(socket, request);
         //re = do_txrx(socket, request, status_code);
      }
   } catch ( invalid_http_request& e ) {
      e.append_log( FC_LOG_MESSAGE( info, "Please verify this url is valid: ${url}", ("url", url.scheme + "://" + url.server + ":" + url.port + url.path) ) );
      e.append_log( FC_LOG_MESSAGE( info, "If the condition persists, please contact the RPC server administrator for ${server}!", ("server", url.server) ) );
      throw;
   }

   std::string response_result = "{}";
   return response_result;
   }
}}}
