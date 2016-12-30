#ifndef __RFC_5646_HPP_INCLUDED__
#define __RFC_5646_HPP_INCLUDED__

/*
Tags for Identifying Languages
https://tools.ietf.org/html/rfc5646

*/

namespace xtd {
	namespace Grammars {
		namespace RFC5646 {
			using namespace xtd::Parser;

#pragma region("forward declerations")

			struct Language_Tag;
			struct langtag;
			struct language;
			struct extlang;
			struct script;
			struct region;
			struct variant;
			struct extension;
			struct singleton;
			struct privateuse;
			struct grandfathered;
			struct irregular;
			struct regular;
#pragma endregion

#pragma region("strings")
			STRING(en_GB_oed, "en-GB-oed");
			STRING(i_ami, "i-ami");
			STRING(i_bnn, "i-bnn");
			STRING(i_default, "i-default");
			STRING(i_enochian, "i-enochian");
			STRING(i_hak, "i-hak");
			STRING(i_klingon, "i-klingon");
			STRING(i_lux, "i-lux");
			STRING(i_mingo, "i-mingo");
			STRING(i_navajo, "i-navajo");
			STRING(i_pwn, "i-pwn");
			STRING(i_tao, "i-tao");
			STRING(i_tay, "i-tay");
			STRING(i_tsu, "i-tsu");
			STRING(sgn_BE_FR, "sgn-BE-FR");
			STRING(sgn_BE_NL, "sgn-BE-NL");
			STRING(sgn_CH_DE, "sgn-CH-DE");
			STRING(art_lojban, "art-lojban");
			STRING(cel_gaulish, "cel-gaulish");
			STRING(no_bok, "no-bok");
			STRING(no_nyn, "no-nyn");
			STRING(zh_guoyu, "zh-guoyu");
			STRING(zh_hakka, "zh-hakka");
			STRING(zh_min, "zh-min");
			STRING(zh_min_nan, "zh-min-nan");
			STRING(zh_xiang, "zh-xiang");
#pragma endregion

#pragma region("imports")
			using ALPHA = MultiByte::Alpha;
			using DIGIT = MultiByte::Digit;
			using alphanum = MultiByte::AlphaNumeric;
#pragma endregion

#pragma region("rules")
			//+ 2.1.  Syntax

			//- Language - Tag = langtag / privateuse / grandfathered
			struct Language_Tag : Rule < Language_Tag, Or<langtag, privateuse, grandfathered> > {};

			//- langtag       = language ["-" script] ["-" region] *("-" variant) *("-" extension) ["-" privateuse]
			struct langtag : Rule < langtag, language, ZeroOrOne<MultiByte::Hyphen, script>, ZeroOrOne<MultiByte::Hyphen, region>, ZeroOrMore<MultiByte::Hyphen, variant>, ZeroOrMore<MultiByte::Hyphen, extension>, ZeroOrOne<MultiByte::Hyphen, privateuse> > {};

			//-language      = 2*3ALPHA ["-" extlang] / 4ALPHA / 5*8ALPHA
			struct language : Rule < language, Or<And< Repeat<ALPHA, 2, 3>, ZeroOrOne<MultiByte::Hyphen, extlang>>, Repeat<ALPHA, 4>, Repeat<ALPHA, 5, 8>> > {};

			//-  extlang       = 3ALPHA *2("-" 3ALPHA); permanently reserved
			struct extlang : Rule < extlang, Repeat<ALPHA, 3>, Repeat<And<MultiByte::Hyphen, Repeat<ALPHA, 3>>, 0, 2> > {};

			//- script        = 4ALPHA
			struct script : Rule < script, Repeat<ALPHA, 4> > {};

			//- region        = 2ALPHA / 3DIGIT
			struct region : Rule < region, Or<Repeat<ALPHA, 2>, Repeat<DIGIT, 3>> > {};

			//-  variant       = 5*8alphanum / (DIGIT 3alphanum)
			struct variant : Rule < variant, Or<Repeat<alphanum, 5, 8>, And<DIGIT, Repeat<alphanum, 3>>> > {};

			//- extension     = singleton 1*("-" (2*8alphanum))
			struct extension : Rule < extension, singleton, OneOrMore<MultiByte::Hyphen, Repeat<alphanum, 2, 8>> > {};

			//- singleton     = DIGIT / %x41-57 / %x59-5A / %x61-77 / %x79-7A
			struct singleton : Rule < singleton, Or<DIGIT, Character<char, 0x41, 0x57>, Character<char, 0x59, 0x5a>, Character<char, 0x61, 0x77>, Character<char, 0x59, 0x7a>> > {};

			//-  privateuse    = "x" 1*("-" (1*8alphanum))
			struct privateuse : Rule < privateuse, MultiByte::_x, OneOrMore<MultiByte::Hyphen, Repeat<alphanum, 1, 8>> > {};

			//-  grandfathered = irregular / regular
			struct grandfathered : Rule < grandfathered, Or<irregular, regular> > {};

			struct irregular : Rule < irregular, Or<en_GB_oed, i_ami, i_bnn, i_default, i_enochian, i_hak, i_klingon, i_lux, i_mingo, i_navajo, i_pwn, i_tao, i_tay, i_tsu, sgn_BE_FR, sgn_BE_NL, sgn_CH_DE> > {};

			struct regular : Rule < regular, Or<art_lojban, cel_gaulish, no_bok, no_nyn, zh_guoyu, zh_hakka, zh_min, zh_min_nan, zh_xiang> > {};
#pragma endregion
		}
	}
}

#endif //__RFC_5646_HPP_INCLUDED__