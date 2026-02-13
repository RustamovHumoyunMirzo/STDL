#pragma once
#include "scene.hpp"
#include <tao/pegtl.hpp>
#include <string>

namespace STDLParser {
namespace pegtl = TAO_PEGTL_NAMESPACE;

namespace grammar {
struct comment : pegtl::seq<pegtl::string<'/','/'>, pegtl::until<pegtl::eolf>> {};
struct ws : pegtl::plus<pegtl::space> {};
struct opt_ws : pegtl::star<pegtl::space> {};

struct integer : pegtl::plus<pegtl::digit> {};
struct floating : pegtl::seq<pegtl::plus<pegtl::digit>, pegtl::one<'.'>, pegtl::plus<pegtl::digit>> {};
struct boolean : pegtl::sor<pegtl::string<'t','r','u','e'>, pegtl::string<'f','a','l','s','e'>> {};
struct quoted_string : pegtl::seq<pegtl::one<'"'>, pegtl::star<pegtl::not_one<'"'>>, pegtl::one<'"'>> {};
struct local_ref : pegtl::seq<pegtl::one<'<'>, pegtl::opt<pegtl::plus<pegtl::not_one<'#'>>>, pegtl::one<'#'>, pegtl::plus<pegtl::digit>, pegtl::one<'>'>> {};
struct global_ref : pegtl::seq<pegtl::one<'<'>, pegtl::opt<pegtl::plus<pegtl::not_one<':'>>>, pegtl::one<':'>, pegtl::plus<pegtl::not_one<'@'>>, pegtl::one<'@'>, pegtl::plus<pegtl::digit>, pegtl::one<'>'>> {};
struct value;
struct list : pegtl::seq<pegtl::one<'['>, opt_ws, pegtl::list<value, pegtl::one<','>>, opt_ws, pegtl::one<']'>> {};
struct value : pegtl::sor<floating, integer, boolean, quoted_string, list, local_ref, global_ref> {};
struct key : pegtl::plus<pegtl::alpha> {};
struct property : pegtl::seq<key, opt_ws, pegtl::one<'='>, opt_ws, value> {};
struct node;
struct nodes : pegtl::star<pegtl::sor<node, property, comment, ws>> {};
struct node_header : pegtl::seq<pegtl::string<'n','o','d','e'>, ws, pegtl::plus<pegtl::not_one<'{', '\n', '\r'>>, opt_ws> {};
struct node : pegtl::seq<node_header, pegtl::one<'{'>, nodes, pegtl::one<'}'>> {};
struct scene : pegtl::seq<pegtl::string<'s','c','e','n','e',' ','v','1'>, pegtl::star<pegtl::sor<ws, comment>>, nodes> {};
}

bool ParseSTDL(const std::string& input, Scene& scene);
}