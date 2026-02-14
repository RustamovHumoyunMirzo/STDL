#pragma once
#include "scene.hpp"
#include <tao/pegtl.hpp>
#include <string>

namespace STDLParser {
namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace grammar {
struct comment : pegtl::seq<pegtl::string<'/','/'>, pegtl::until<pegtl::eolf>> {};
struct ws : pegtl::plus<pegtl::space> {};
struct opt_ws : pegtl::star<pegtl::sor<pegtl::space, comment>> {};
struct ws_or_comment : pegtl::sor<ws, comment> {};
struct opt_ws_or_comment : pegtl::star<pegtl::sor<pegtl::space, comment>> {};

struct sign : pegtl::opt<pegtl::one<'-', '+'>> {};
struct integer : pegtl::seq<sign, pegtl::plus<pegtl::digit>> {};
struct floating : pegtl::seq<sign, pegtl::plus<pegtl::digit>, pegtl::one<'.'>, pegtl::plus<pegtl::digit>> {};
struct boolean : pegtl::sor<pegtl::string<'t','r','u','e'>, pegtl::string<'f','a','l','s','e'>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, pegtl::star<pegtl::not_one<'"'>>, pegtl::one<'"'>> {};
struct local_ref : pegtl::seq<pegtl::one<'<'>, pegtl::opt<pegtl::plus<pegtl::not_one<'#', '>', ',', '\n', '\r'>>>, pegtl::one<'#'>, pegtl::plus<pegtl::digit>, pegtl::one<'>'>> {};
struct global_ref : pegtl::seq<pegtl::one<'<'>, pegtl::plus<pegtl::not_one<':', '>', ',', '\n', '\r'>>, pegtl::one<':'>, pegtl::plus<pegtl::not_one<'@', '>', ',', '\n', '\r'>>, pegtl::one<'@'>, pegtl::plus<pegtl::digit>, pegtl::one<'>'>> {};
struct value;
struct list_element : pegtl::seq<opt_ws, value> {};
struct list_separator : pegtl::seq<opt_ws, pegtl::one<','>, opt_ws> {};
struct list_content : pegtl::seq<list_element, pegtl::star<pegtl::seq<list_separator, list_element>>> {};
struct list : pegtl::seq<pegtl::one<'['>, pegtl::opt<list_content>, opt_ws, pegtl::one<']'>> {};
struct value : pegtl::sor<floating, integer, boolean, quoted_string, list, local_ref, global_ref> {};
struct key : pegtl::seq<pegtl::alpha, pegtl::star<pegtl::sor<pegtl::alnum, pegtl::one<'_'>>>> {};
struct property : pegtl::seq<key, opt_ws, pegtl::one<'='>, opt_ws, value> {};
struct node_header;
struct node;
struct nodes : pegtl::star<pegtl::sor<node, property, ws_or_comment>> {};
struct node_header : pegtl::seq<pegtl::string<'n','o','d','e'>, ws, pegtl::plus<pegtl::not_one<'{', '\n', '\r'>>, opt_ws_or_comment> {};
struct node : pegtl::seq<node_header, pegtl::one<'{'>, opt_ws_or_comment, nodes, opt_ws_or_comment, pegtl::one<'}'>> {};
struct scene : pegtl::seq<pegtl::string<'s','c','e','n','e',' ','v','1'>, opt_ws_or_comment, pegtl::star<pegtl::sor<node, ws_or_comment>>, opt_ws_or_comment, pegtl::eof> {};
}

bool ParseSTDL(const std::string& input, Scene& scene);
}