/*
 * Covariant Script Unicode Extension
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright (C) 2017-2023 Michael Lee(李登淳)
 *
 * Email:   lee@covariant.cn, mikecovlee@163.com
 * Github:  https://github.com/mikecovlee
 * Website: http://covscript.org.cn
 */
#include <covscript/cni.hpp>
#include <covscript/dll.hpp>
#include <codecvt>
#include <cwctype>
#include <regex>

using uwstring_t = std::u32string;
using uwchar_t = char32_t;

namespace codecvt_impl {
#if COVSCRIPT_ABI_VERSION < 251108
	using namespace cs;

	class charset {
	public:
		virtual ~charset() = default;

		virtual uwstring_t local2wide(const std::string &) = 0;

		virtual std::string wide2local(const uwstring_t &) = 0;

		virtual bool is_identifier(uwchar_t) = 0;
	};

	class ascii final : public charset {
	public:
		uwstring_t local2wide(const std::string &local) override
		{
			return uwstring_t(local.begin(), local.end());
		}

		std::string wide2local(const uwstring_t &str) override
		{
			std::string local;
			local.reserve(str.size());
			for (auto ch : str) local.push_back(ch);
			return std::move(local);
		}

		bool is_identifier(uwchar_t ch) override
		{
			return ch == '_' || std::iswalnum(ch);
		}
	};

	class utf8 final : public charset {
		std::wstring_convert<std::codecvt_utf8<uwchar_t>, uwchar_t> cvt;

		static constexpr std::uint32_t ascii_max = 0x7F;

	public:
		uwstring_t local2wide(const std::string &str) override
		{
			return cvt.from_bytes(str);
		}

		std::string wide2local(const uwstring_t &str) override
		{
			return cvt.to_bytes(str);
		}

		bool is_identifier(uwchar_t ch) override
		{
			/**
			 * Chinese Character in Unicode Charset
			 * Basic:    0x4E00 - 0x9FA5
			 * Extended: 0x9FA6 - 0x9FEF
			 * Special:  0x3007
			 */
			if (ch > ascii_max)
				return (ch >= 0x4E00 && ch <= 0x9FA5) || (ch >= 0x9FA6 && ch <= 0x9FEF) ||
				       ch == 0x3007;
			else
				return ch == '_' || std::iswalnum(ch);
		}
	};

	class gbk final : public charset {
		static inline uwchar_t set_zero(uwchar_t ch)
		{
			return ch & 0x0000ffff;
		}

		static constexpr std::uint8_t u8_blck_begin = 0x80;
		static constexpr std::uint32_t u32_blck_begin = 0x8000;

	public:
		uwstring_t local2wide(const std::string &local) override
		{
			uwstring_t wide;
			uint32_t head = 0;
			bool read_next = true;
			for (auto it = local.begin(); it != local.end();) {
				if (read_next) {
					head = *(it++);
					if (head & u8_blck_begin)
						read_next = false;
					else
						wide.push_back(set_zero(head));
				}
				else {
					std::uint8_t tail = *(it++);
					wide.push_back(set_zero(head << 8 | tail));
					read_next = true;
				}
			}
			if (!read_next) throw compile_error("Codecvt: Bad encoding.");
			return std::move(wide);
		}

		std::string wide2local(const uwstring_t &wide) override
		{
			std::string local;
			for (auto &ch : wide) {
				if (ch & u32_blck_begin) local.push_back(ch >> 8);
				local.push_back(ch);
			}
			return std::move(local);
		}

		bool is_identifier(uwchar_t ch) override
		{
			/**
			 * Chinese Character in GBK Charset
			 * GBK/2: 0xB0A1 - 0xF7FE
			 * GBK/3: 0x8140 - 0xA0FE
			 * GBK/4: 0xAA40 - 0xFEA0
			 * GBK/5: 0xA996
			 */
			if (ch & u32_blck_begin)
				return (ch >= 0xB0A1 && ch <= 0xF7FE) || (ch >= 0x8140 && ch <= 0xA0FE) ||
				       (ch >= 0xAA40 && ch <= 0xFEA0) || ch == 0xA996;
			else
				return ch == '_' || std::iswalnum(ch);
		}
	};
#else
	using namespace cs::codecvt;
#endif
}  // namespace codecvt_impl

using codecvt_t = std::shared_ptr<codecvt_impl::charset>;
using wregex_t = std::basic_regex<uwchar_t>;
using wsmatch_t = std::match_results<std::u32string::const_iterator>;

CNI_ROOT_NAMESPACE {
	using namespace cs;

	CNI_NAMESPACE(codecvt)
	{
		cs::var make_codecvt_ascii() {
			return codecvt_t(new codecvt_impl::ascii);
		}

		CNI_REGISTER(ascii,
		             cs::var::make_constant<cs::type_t>(make_codecvt_ascii,
		                     cs::type_id(typeid(codecvt_t))))

		cs::var make_codecvt_utf8() {
			return codecvt_t(new codecvt_impl::utf8);
		}

		CNI_REGISTER(utf8,
		             cs::var::make_constant<cs::type_t>(make_codecvt_utf8,
		                     cs::type_id(typeid(codecvt_t))))

		cs::var make_codecvt_gbk() {
			return codecvt_t(new codecvt_impl::gbk);
		}

		CNI_REGISTER(gbk,
		             cs::var::make_constant<cs::type_t>(make_codecvt_gbk,
		                     cs::type_id(typeid(codecvt_t))))

		uwstring_t local2wide(const codecvt_t &cvt, const std::string &str) {
			return cvt->local2wide(str);
		}

		CNI(local2wide)

		std::string wide2local(const codecvt_t &cvt, const uwstring_t &str) {
			return cvt->wide2local(str);
		}

		CNI(wide2local)

		bool is_identifier(const codecvt_t &cvt, uwchar_t ch) {
			return cvt->is_identifier(ch);
		}

		CNI(is_identifier)
	}

	CNI_NAMESPACE(wchar)
	{
		bool isalnum(uwchar_t c) {
			return std::iswalnum(c);
		}

		CNI(isalnum)

		bool isalpha(uwchar_t c) {
			return std::iswalpha(c);
		}

		CNI(isalpha)

		bool islower(uwchar_t c) {
			return std::iswlower(c);
		}

		CNI(islower)

		bool isupper(uwchar_t c) {
			return std::iswupper(c);
		}

		CNI(isupper)

		bool isdigit(uwchar_t c) {
			return std::iswdigit(c);
		}

		CNI(isdigit)

		bool iscntrl(uwchar_t c) {
			return std::iswcntrl(c);
		}

		CNI(iscntrl)

		bool isgraph(uwchar_t c) {
			return std::iswgraph(c);
		}

		CNI(isgraph)

		bool isspace(uwchar_t c) {
			return std::iswspace(c);
		}

		CNI(isspace)

		bool isblank(uwchar_t c) {
			return std::iswblank(c);
		}

		CNI(isblank)

		bool isprint(uwchar_t c) {
			return std::iswprint(c);
		}

		CNI(isprint)

		bool ispunct(uwchar_t c) {
			return std::iswpunct(c);
		}

		CNI(ispunct)

		uwchar_t tolower(uwchar_t c) {
			return std::towlower(c);
		}

		CNI(tolower)

		uwchar_t toupper(uwchar_t c) {
			return std::towupper(c);
		}

		CNI(toupper)

		uwchar_t from_char(char c) {
			return c;
		}

		CNI(from_char)

		uwchar_t from_unicode(const numeric &unicode) {
			if (unicode.as_integer() < 0) throw lang_error("Out of range.");
			return static_cast<uwchar_t>(unicode.as_integer());
		}

		CNI(from_unicode)

		uwstring_t to_wstring(uwchar_t c) {
			return uwstring_t(1, c);
		}

		CNI(to_wstring)
	}

	CNI_NAMESPACE(wstring_type)
	{
		uwchar_t at(const uwstring_t &str, numeric idx) {
			return str.at(idx.as_integer());
		}

		CNI(at)

		uwstring_t assign(uwstring_t &str, numeric posit, char ch) {
			str.at(posit.as_integer()) = ch;
			return str;
		}

		CNI(assign)

		uwstring_t append(uwstring_t &str, const uwstring_t &val) {
			str.append(val);
			return str;
		}

		CNI(append)

		uwstring_t insert(uwstring_t &str, numeric posit, const uwstring_t &val) {
			str.insert(posit.as_integer(), val);
			return str;
		}

		CNI(insert)

		uwstring_t erase(uwstring_t &str, numeric b, numeric e) {
			str.erase(b.as_integer(), e.as_integer());
			return str;
		}

		CNI(erase)

		uwstring_t replace(uwstring_t &str, numeric posit, numeric count,
		                   const uwstring_t &val) {
			str.replace(posit.as_integer(), count.as_integer(), val);
			return str;
		}

		CNI(replace)

		uwstring_t substr(const uwstring_t &str, numeric b, numeric e) {
			return str.substr(b.as_integer(), e.as_integer());
		}

		CNI(substr)

		numeric find(const uwstring_t &str, const uwstring_t &s, numeric posit) {
			auto pos = str.find(s, posit.as_integer());
			if (pos == uwstring_t::npos)
				return -1;
			else
				return pos;
		}

		CNI(find)

		numeric rfind(const uwstring_t &str, const uwstring_t &s, numeric posit) {
			std::size_t pos = 0;
			if (posit.as_integer() == -1)
				pos = str.rfind(s, uwstring_t::npos);
			else
				pos = str.rfind(s, posit.as_integer());
			if (pos == uwstring_t::npos)
				return -1;
			else
				return pos;
		}

		CNI(rfind)

		uwstring_t cut(uwstring_t &str, numeric n) {
			for (std::size_t i = 0; i < n.as_integer(); ++i) str.pop_back();
			return str;
		}

		CNI(cut)

		bool empty(const uwstring_t &str) {
			return str.empty();
		}

		CNI(empty)

		void clear(uwstring_t &str) {
			str.clear();
		}

		CNI(clear)

		numeric size(const uwstring_t &str) {
			return str.size();
		}

		CNI_VISITOR(size)

		uwstring_t tolower(const uwstring_t &str) {
			uwstring_t s;
			for (auto &ch : str) s.push_back(std::towlower(ch));
			return std::move(s);
		}

		CNI(tolower)

		uwstring_t toupper(const uwstring_t &str) {
			uwstring_t s;
			for (auto &ch : str) s.push_back(std::towupper(ch));
			return std::move(s);
		}

		CNI(toupper)

		numeric to_number(const uwstring_t &str, const codecvt_t &cvt) {
			return parse_number(cvt->wide2local(str));
		}

		CNI(to_number)

		array split(const uwstring_t &str, const array &signals) {
			array arr;
			uwstring_t buf;
			bool found = false;
			for (auto &ch : str) {
				for (auto &sig : signals) {
					if (sig.type() == typeid(char) && ch == sig.const_val<char>()) {
						if (!buf.empty()) {
							arr.push_back(buf);
							buf.clear();
						}
						found = true;
						break;
					}
					else if (sig.type() == typeid(uwchar_t) &&
					         ch == sig.const_val<uwchar_t>()) {
						if (!buf.empty()) {
							arr.push_back(buf);
							buf.clear();
						}
						found = true;
						break;
					}
				}
				if (found)
					found = false;
				else
					buf.push_back(ch);
			}
			if (!buf.empty()) arr.push_back(buf);
			return std::move(arr);
		}

		CNI(split)
	}

	CNI_NAMESPACE(wregex)
	{
		wsmatch_t match(wregex_t & reg, const uwstring_t &str) {
			wsmatch_t m;
			std::regex_search(str, m, reg);
			return std::move(m);
		}

		CNI(match)

		wsmatch_t search(wregex_t &reg, const uwstring_t &str) {
			wsmatch_t m;
			std::regex_search(str, m, reg);
			return std::move(m);
		}

		CNI(search)

		uwstring_t replace(wregex_t &reg, const uwstring_t &str,
		                   const uwstring_t &fmt) {
			return std::regex_replace(str, reg, fmt);
		}

		CNI(replace)
	}

	CNI_NAMESPACE(wsmatch)
	{
		bool ready(const wsmatch_t &m) {
			return m.ready();
		}

		CNI(ready)

		bool empty(const wsmatch_t &m) {
			return m.empty();
		}

		CNI(empty)

		numeric size(const wsmatch_t &m) {
			return m.size();
		}

		CNI(size)

		numeric length(const wsmatch_t &m, numeric index) {
			return m.length(index.as_integer());
		}

		CNI(length)

		numeric position(const wsmatch_t &m, numeric index) {
			return m.position(index.as_integer());
		}

		CNI(position)

		uwstring_t str(const wsmatch_t &m, numeric index) {
			return m.str(index.as_integer());
		}

		CNI(str)

		uwstring_t prefix(const wsmatch_t &m) {
			return m.prefix().str();
		}

		CNI(prefix)

		uwstring_t suffix(const wsmatch_t &m) {
			return m.suffix().str();
		}

		CNI(suffix)
	}

	var make_wstring()
	{
		return cs::var::make<uwstring_t>();
	}

	CNI_REGISTER(wstring, var::make_constant<cs::type_t>(
	                 make_wstring, type_id(typeid(uwstring_t))))

	var build_wregex(const uwstring_t &str)
	{
		return var::make<wregex_t>(str);
	}

	CNI(build_wregex)

	var build_optimize_wregex(const uwstring_t &str)
	{
		return var::make<wregex_t>(str, std::regex_constants::optimize);
	}

	CNI(build_optimize_wregex)
}

CNI_ENABLE_TYPE_EXT_V(codecvt, codecvt_t, "unicode::codecvt")
CNI_ENABLE_TYPE_EXT_V(wchar, uwchar_t, "unicode::wchar")
CNI_ENABLE_TYPE_EXT_V(wstring_type, uwstring_t, "unicode::wstring")
CNI_ENABLE_TYPE_EXT_V(wregex, wregex_t, "unicode::wregex")
CNI_ENABLE_TYPE_EXT_V(wsmatch, wsmatch_t, "unicode::wregex::result")
