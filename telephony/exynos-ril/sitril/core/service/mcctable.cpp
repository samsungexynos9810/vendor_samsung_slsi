/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * mcctable.cpp
 *
 *  Created on: 2014. 10. 24.
 *      Author: sungwoo48.choi
 */

#include <cutils/properties.h>
#include "mcctable.h"
#include "textutils.h"
#include "rillog.h"

// attach PDN connection type
#define PROPERTY_TEST_ESM_ZERO    "persist.vendor.ril.test.esmzeroflag"

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_ETC, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

// MccEntry
MccEntry::MccEntry(int mcc, const char *iso, int smallestDigitsMcc)
{
    this->mcc = mcc;
    this->iso = iso;
    this->smallestDigitsMcc = smallestDigitsMcc;
    this->language = NULL;
}

MccEntry::MccEntry(int mcc, const char *iso, int smallestDigitsMcc, const char *language)
{
    this->mcc = mcc;
    this->iso = iso;
    this->smallestDigitsMcc = smallestDigitsMcc;
    this->language = language;
}

IMPLEMENT_MODULE_TAG(MccTable, MccTable)

// MccTable
// defined in MccTable.java in Android telephony framework
// SHOULD be synchronized on every updated
MccTable::MccTable()
{
    InitMccTable();
    InitEsmZeroFlagNetworkList();
    InitNotUsePnnOplOperator();
}

MccTable::~MccTable()
{
    for (map<int, MccEntry *>::iterator iter = m_mcctable.begin(); iter != m_mcctable.end(); ++iter) {
        MccEntry *entry = iter->second;
        if (entry != NULL)
            delete entry;
    } // end for i ~
    m_mcctable.clear();
}

void MccTable::InitMccTable()
{
    AddEntry(new MccEntry(202,"gr",2));    //Greece
    AddEntry(new MccEntry(204,"nl",2,"nl"));    //Netherlands (Kingdom of the)
    AddEntry(new MccEntry(206,"be",2));    //Belgium
    AddEntry(new MccEntry(208,"fr",2,"fr"));    //France
    AddEntry(new MccEntry(212,"mc",2));    //Monaco (Principality of)
    AddEntry(new MccEntry(213,"ad",2));    //Andorra (Principality of)
    AddEntry(new MccEntry(214,"es",2,"es"));    //Spain
    AddEntry(new MccEntry(216,"hu",2));    //Hungary (Republic of)
    AddEntry(new MccEntry(218,"ba",2));    //Bosnia and Herzegovina
    AddEntry(new MccEntry(219,"hr",2));    //Croatia (Republic of)
    AddEntry(new MccEntry(220,"rs",2));    //Serbia and Montenegro
    AddEntry(new MccEntry(222,"it",2,"it"));    //Italy
    AddEntry(new MccEntry(225,"va",2,"it"));    //Vatican City State
    AddEntry(new MccEntry(226,"ro",2));    //Romania
    AddEntry(new MccEntry(228,"ch",2,"de"));    //Switzerland (Confederation of)
    AddEntry(new MccEntry(230,"cz",2,"cs"));    //Czech Republic
    AddEntry(new MccEntry(231,"sk",2));    //Slovak Republic
    AddEntry(new MccEntry(232,"at",2,"de"));    //Austria
    AddEntry(new MccEntry(234,"gb",2,"en"));    //United Kingdom of Great Britain and Northern Ireland
    AddEntry(new MccEntry(235,"gb",2,"en"));    //United Kingdom of Great Britain and Northern Ireland
    AddEntry(new MccEntry(238,"dk",2));    //Denmark
    AddEntry(new MccEntry(240,"se",2,"sv"));    //Sweden
    AddEntry(new MccEntry(242,"no",2));    //Norway
    AddEntry(new MccEntry(244,"fi",2));    //Finland
    AddEntry(new MccEntry(246,"lt",2));    //Lithuania (Republic of)
    AddEntry(new MccEntry(247,"lv",2));    //Latvia (Republic of)
    AddEntry(new MccEntry(248,"ee",2));    //Estonia (Republic of)
    AddEntry(new MccEntry(250,"ru",2));    //Russian Federation
    AddEntry(new MccEntry(255,"ua",2));    //Ukraine
    AddEntry(new MccEntry(257,"by",2));    //Belarus (Republic of)
    AddEntry(new MccEntry(259,"md",2));    //Moldova (Republic of)
    AddEntry(new MccEntry(260,"pl",2));    //Poland (Republic of)
    AddEntry(new MccEntry(262,"de",2,"de"));    //Germany (Federal Republic of)
    AddEntry(new MccEntry(266,"gi",2));    //Gibraltar
    AddEntry(new MccEntry(268,"pt",2));    //Portugal
    AddEntry(new MccEntry(270,"lu",2));    //Luxembourg
    AddEntry(new MccEntry(272,"ie",2,"en"));    //Ireland
    AddEntry(new MccEntry(274,"is",2));    //Iceland
    AddEntry(new MccEntry(276,"al",2));    //Albania (Republic of)
    AddEntry(new MccEntry(278,"mt",2));    //Malta
    AddEntry(new MccEntry(280,"cy",2));    //Cyprus (Republic of)
    AddEntry(new MccEntry(282,"ge",2));    //Georgia
    AddEntry(new MccEntry(283,"am",2));    //Armenia (Republic of)
    AddEntry(new MccEntry(284,"bg",2));    //Bulgaria (Republic of)
    AddEntry(new MccEntry(286,"tr",2));    //Turkey
    AddEntry(new MccEntry(288,"fo",2));    //Faroe Islands
    AddEntry(new MccEntry(289,"ge",2));    //Abkhazia (Georgia)
    AddEntry(new MccEntry(290,"gl",2));    //Greenland (Denmark)
    AddEntry(new MccEntry(292,"sm",2));    //San Marino (Republic of)
    AddEntry(new MccEntry(293,"si",2));    //Slovenia (Republic of)
    AddEntry(new MccEntry(294,"mk",2));   //The Former Yugoslav Republic of Macedonia
    AddEntry(new MccEntry(295,"li",2));    //Liechtenstein (Principality of)
    AddEntry(new MccEntry(297,"me",2));    //Montenegro (Republic of)
    AddEntry(new MccEntry(302,"ca",3,"en"));    //Canada
    AddEntry(new MccEntry(308,"pm",2));    //Saint Pierre and Miquelon (Collectivit territoriale de la Rpublique franaise)
    AddEntry(new MccEntry(310,"us",3,"en"));    //United States of America
    AddEntry(new MccEntry(311,"us",3,"en"));    //United States of America
    AddEntry(new MccEntry(312,"us",3,"en"));    //United States of America
    AddEntry(new MccEntry(313,"us",3,"en"));    //United States of America
    AddEntry(new MccEntry(314,"us",3,"en"));    //United States of America
    AddEntry(new MccEntry(315,"us",3,"en"));    //United States of America
    AddEntry(new MccEntry(316,"us",3,"en"));    //United States of America
    AddEntry(new MccEntry(330,"pr",2));    //Puerto Rico
    AddEntry(new MccEntry(332,"vi",2));    //United States Virgin Islands
    AddEntry(new MccEntry(334,"mx",3));    //Mexico
    AddEntry(new MccEntry(338,"jm",3));    //Jamaica
    AddEntry(new MccEntry(340,"gp",2));    //Guadeloupe (French Department of)
    AddEntry(new MccEntry(342,"bb",3));    //Barbados
    AddEntry(new MccEntry(344,"ag",3));    //Antigua and Barbuda
    AddEntry(new MccEntry(346,"ky",3));    //Cayman Islands
    AddEntry(new MccEntry(348,"vg",3));    //British Virgin Islands
    AddEntry(new MccEntry(350,"bm",2));    //Bermuda
    AddEntry(new MccEntry(352,"gd",2));    //Grenada
    AddEntry(new MccEntry(354,"ms",2));    //Montserrat
    AddEntry(new MccEntry(356,"kn",2));    //Saint Kitts and Nevis
    AddEntry(new MccEntry(358,"lc",2));    //Saint Lucia
    AddEntry(new MccEntry(360,"vc",2));    //Saint Vincent and the Grenadines
    AddEntry(new MccEntry(362,"ai",2));    //Netherlands Antilles
    AddEntry(new MccEntry(363,"aw",2));    //Aruba
    AddEntry(new MccEntry(364,"bs",2));    //Bahamas (Commonwealth of the)
    AddEntry(new MccEntry(365,"ai",3));    //Anguilla
    AddEntry(new MccEntry(366,"dm",2));    //Dominica (Commonwealth of)
    AddEntry(new MccEntry(368,"cu",2));    //Cuba
    AddEntry(new MccEntry(370,"do",2));    //Dominican Republic
    AddEntry(new MccEntry(372,"ht",2));    //Haiti (Republic of)
    AddEntry(new MccEntry(374,"tt",2));    //Trinidad and Tobago
    AddEntry(new MccEntry(376,"tc",2));    //Turks and Caicos Islands
    AddEntry(new MccEntry(400,"az",2));    //Azerbaijani Republic
    AddEntry(new MccEntry(401,"kz",2));    //Kazakhstan (Republic of)
    AddEntry(new MccEntry(402,"bt",2));    //Bhutan (Kingdom of)
    AddEntry(new MccEntry(404,"in",2));    //India (Republic of)
    AddEntry(new MccEntry(405,"in",2));    //India (Republic of)
    AddEntry(new MccEntry(410,"pk",2));    //Pakistan (Islamic Republic of)
    AddEntry(new MccEntry(412,"af",2));    //Afghanistan
    AddEntry(new MccEntry(413,"lk",2));    //Sri Lanka (Democratic Socialist Republic of)
    AddEntry(new MccEntry(414,"mm",2));    //Myanmar (Union of)
    AddEntry(new MccEntry(415,"lb",2));    //Lebanon
    AddEntry(new MccEntry(416,"jo",2));    //Jordan (Hashemite Kingdom of)
    AddEntry(new MccEntry(417,"sy",2));    //Syrian Arab Republic
    AddEntry(new MccEntry(418,"iq",2));    //Iraq (Republic of)
    AddEntry(new MccEntry(419,"kw",2));    //Kuwait (State of)
    AddEntry(new MccEntry(420,"sa",2));    //Saudi Arabia (Kingdom of)
    AddEntry(new MccEntry(421,"ye",2));    //Yemen (Republic of)
    AddEntry(new MccEntry(422,"om",2));    //Oman (Sultanate of)
    AddEntry(new MccEntry(423,"ps",2));    //Palestine
    AddEntry(new MccEntry(424,"ae",2));    //United Arab Emirates
    AddEntry(new MccEntry(425,"il",2));    //Israel (State of)
    AddEntry(new MccEntry(426,"bh",2));    //Bahrain (Kingdom of)
    AddEntry(new MccEntry(427,"qa",2));    //Qatar (State of)
    AddEntry(new MccEntry(428,"mn",2));    //Mongolia
    AddEntry(new MccEntry(429,"np",2));    //Nepal
    AddEntry(new MccEntry(430,"ae",2));    //United Arab Emirates
    AddEntry(new MccEntry(431,"ae",2));    //United Arab Emirates
    AddEntry(new MccEntry(432,"ir",2));    //Iran (Islamic Republic of)
    AddEntry(new MccEntry(434,"uz",2));    //Uzbekistan (Republic of)
    AddEntry(new MccEntry(436,"tj",2));    //Tajikistan (Republic of)
    AddEntry(new MccEntry(437,"kg",2));    //Kyrgyz Republic
    AddEntry(new MccEntry(438,"tm",2));    //Turkmenistan
    AddEntry(new MccEntry(440,"jp",2,"ja"));    //Japan
    AddEntry(new MccEntry(441,"jp",2,"ja"));    //Japan
    AddEntry(new MccEntry(450,"kr",2,"ko"));    //Korea (Republic of)
    AddEntry(new MccEntry(452,"vn",2));    //Viet Nam (Socialist Republic of)
    AddEntry(new MccEntry(454,"hk",2));    //"Hong Kong, China"
    AddEntry(new MccEntry(455,"mo",2));    //"Macao, China"
    AddEntry(new MccEntry(456,"kh",2));    //Cambodia (Kingdom of)
    AddEntry(new MccEntry(457,"la",2));    //Lao People's Democratic Republic
    AddEntry(new MccEntry(460,"cn",2,"zh"));    //China (People's Republic of)
    AddEntry(new MccEntry(461,"cn",2,"zh"));    //China (People's Republic of)
    AddEntry(new MccEntry(466,"tw",2));    //"Taiwan, China"
    AddEntry(new MccEntry(467,"kp",2));    //Democratic People's Republic of Korea
    AddEntry(new MccEntry(470,"bd",2));    //Bangladesh (People's Republic of)
    AddEntry(new MccEntry(472,"mv",2));    //Maldives (Republic of)
    AddEntry(new MccEntry(502,"my",2));    //Malaysia
    AddEntry(new MccEntry(505,"au",2,"en"));    //Australia
    AddEntry(new MccEntry(510,"id",2));    //Indonesia (Republic of)
    AddEntry(new MccEntry(514,"tl",2));    //Democratic Republic of Timor-Leste
    AddEntry(new MccEntry(515,"ph",2));    //Philippines (Republic of the)
    AddEntry(new MccEntry(520,"th",2));    //Thailand
    AddEntry(new MccEntry(525,"sg",2,"en"));    //Singapore (Republic of)
    AddEntry(new MccEntry(528,"bn",2));    //Brunei Darussalam
    AddEntry(new MccEntry(530,"nz",2, "en"));    //New Zealand
    AddEntry(new MccEntry(534,"mp",2));    //Northern Mariana Islands (Commonwealth of the)
    AddEntry(new MccEntry(535,"gu",2));    //Guam
    AddEntry(new MccEntry(536,"nr",2));    //Nauru (Republic of)
    AddEntry(new MccEntry(537,"pg",2));    //Papua New Guinea
    AddEntry(new MccEntry(539,"to",2));    //Tonga (Kingdom of)
    AddEntry(new MccEntry(540,"sb",2));    //Solomon Islands
    AddEntry(new MccEntry(541,"vu",2));    //Vanuatu (Republic of)
    AddEntry(new MccEntry(542,"fj",2));    //Fiji (Republic of)
    AddEntry(new MccEntry(543,"wf",2));    //Wallis and Futuna (Territoire franais d'outre-mer)
    AddEntry(new MccEntry(544,"as",2));    //American Samoa
    AddEntry(new MccEntry(545,"ki",2));    //Kiribati (Republic of)
    AddEntry(new MccEntry(546,"nc",2));    //New Caledonia (Territoire franais d'outre-mer)
    AddEntry(new MccEntry(547,"pf",2));    //French Polynesia (Territoire franais d'outre-mer)
    AddEntry(new MccEntry(548,"ck",2));    //Cook Islands
    AddEntry(new MccEntry(549,"ws",2));    //Samoa (Independent State of)
    AddEntry(new MccEntry(550,"fm",2));    //Micronesia (Federated States of)
    AddEntry(new MccEntry(551,"mh",2));    //Marshall Islands (Republic of the)
    AddEntry(new MccEntry(552,"pw",2));    //Palau (Republic of)
    AddEntry(new MccEntry(602,"eg",2));    //Egypt (Arab Republic of)
    AddEntry(new MccEntry(603,"dz",2));    //Algeria (People's Democratic Republic of)
    AddEntry(new MccEntry(604,"ma",2));    //Morocco (Kingdom of)
    AddEntry(new MccEntry(605,"tn",2));    //Tunisia
    AddEntry(new MccEntry(606,"ly",2));    //Libya (Socialist People's Libyan Arab Jamahiriya)
    AddEntry(new MccEntry(607,"gm",2));    //Gambia (Republic of the)
    AddEntry(new MccEntry(608,"sn",2));    //Senegal (Republic of)
    AddEntry(new MccEntry(609,"mr",2));    //Mauritania (Islamic Republic of)
    AddEntry(new MccEntry(610,"ml",2));    //Mali (Republic of)
    AddEntry(new MccEntry(611,"gn",2));    //Guinea (Republic of)
    AddEntry(new MccEntry(612,"ci",2));    //Cte d'Ivoire (Republic of)
    AddEntry(new MccEntry(613,"bf",2));    //Burkina Faso
    AddEntry(new MccEntry(614,"ne",2));    //Niger (Republic of the)
    AddEntry(new MccEntry(615,"tg",2));    //Togolese Republic
    AddEntry(new MccEntry(616,"bj",2));    //Benin (Republic of)
    AddEntry(new MccEntry(617,"mu",2));    //Mauritius (Republic of)
    AddEntry(new MccEntry(618,"lr",2));    //Liberia (Republic of)
    AddEntry(new MccEntry(619,"sl",2));    //Sierra Leone
    AddEntry(new MccEntry(620,"gh",2));    //Ghana
    AddEntry(new MccEntry(621,"ng",2));    //Nigeria (Federal Republic of)
    AddEntry(new MccEntry(622,"td",2));    //Chad (Republic of)
    AddEntry(new MccEntry(623,"cf",2));    //Central African Republic
    AddEntry(new MccEntry(624,"cm",2));    //Cameroon (Republic of)
    AddEntry(new MccEntry(625,"cv",2));    //Cape Verde (Republic of)
    AddEntry(new MccEntry(626,"st",2));    //Sao Tome and Principe (Democratic Republic of)
    AddEntry(new MccEntry(627,"gq",2));    //Equatorial Guinea (Republic of)
    AddEntry(new MccEntry(628,"ga",2));    //Gabonese Republic
    AddEntry(new MccEntry(629,"cg",2));    //Congo (Republic of the)
    AddEntry(new MccEntry(630,"cg",2));    //Democratic Republic of the Congo
    AddEntry(new MccEntry(631,"ao",2));    //Angola (Republic of)
    AddEntry(new MccEntry(632,"gw",2));    //Guinea-Bissau (Republic of)
    AddEntry(new MccEntry(633,"sc",2));    //Seychelles (Republic of)
    AddEntry(new MccEntry(634,"sd",2));    //Sudan (Republic of the)
    AddEntry(new MccEntry(635,"rw",2));    //Rwanda (Republic of)
    AddEntry(new MccEntry(636,"et",2));    //Ethiopia (Federal Democratic Republic of)
    AddEntry(new MccEntry(637,"so",2));    //Somali Democratic Republic
    AddEntry(new MccEntry(638,"dj",2));    //Djibouti (Republic of)
    AddEntry(new MccEntry(639,"ke",2));    //Kenya (Republic of)
    AddEntry(new MccEntry(640,"tz",2));    //Tanzania (United Republic of)
    AddEntry(new MccEntry(641,"ug",2));    //Uganda (Republic of)
    AddEntry(new MccEntry(642,"bi",2));    //Burundi (Republic of)
    AddEntry(new MccEntry(643,"mz",2));    //Mozambique (Republic of)
    AddEntry(new MccEntry(645,"zm",2));    //Zambia (Republic of)
    AddEntry(new MccEntry(646,"mg",2));    //Madagascar (Republic of)
    AddEntry(new MccEntry(647,"re",2));    //Reunion (French Department of)
    AddEntry(new MccEntry(648,"zw",2));    //Zimbabwe (Republic of)
    AddEntry(new MccEntry(649,"na",2));    //Namibia (Republic of)
    AddEntry(new MccEntry(650,"mw",2));    //Malawi
    AddEntry(new MccEntry(651,"ls",2));    //Lesotho (Kingdom of)
    AddEntry(new MccEntry(652,"bw",2));    //Botswana (Republic of)
    AddEntry(new MccEntry(653,"sz",2));    //Swaziland (Kingdom of)
    AddEntry(new MccEntry(654,"km",2));    //Comoros (Union of the)
    AddEntry(new MccEntry(655,"za",2,"en"));    //South Africa (Republic of)
    AddEntry(new MccEntry(657,"er",2));    //Eritrea
    AddEntry(new MccEntry(702,"bz",2));    //Belize
    AddEntry(new MccEntry(704,"gt",2));    //Guatemala (Republic of)
    AddEntry(new MccEntry(706,"sv",2));    //El Salvador (Republic of)
    AddEntry(new MccEntry(708,"hn",3));    //Honduras (Republic of)
    AddEntry(new MccEntry(710,"ni",2));    //Nicaragua
    AddEntry(new MccEntry(712,"cr",2));    //Costa Rica
    AddEntry(new MccEntry(714,"pa",2));    //Panama (Republic of)
    AddEntry(new MccEntry(716,"pe",2));    //Peru
    AddEntry(new MccEntry(722,"ar",3));    //Argentine Republic
    AddEntry(new MccEntry(724,"br",2));    //Brazil (Federative Republic of)
    AddEntry(new MccEntry(730,"cl",2));    //Chile
    AddEntry(new MccEntry(732,"co",3));    //Colombia (Republic of)
    AddEntry(new MccEntry(734,"ve",2));    //Venezuela (Bolivarian Republic of)
    AddEntry(new MccEntry(736,"bo",2));    //Bolivia (Republic of)
    AddEntry(new MccEntry(738,"gy",2));    //Guyana
    AddEntry(new MccEntry(740,"ec",2));    //Ecuador
    AddEntry(new MccEntry(742,"gf",2));    //French Guiana (French Department of)
    AddEntry(new MccEntry(744,"py",2));    //Paraguay (Republic of)
    AddEntry(new MccEntry(746,"sr",2));    //Suriname (Republic of)
    AddEntry(new MccEntry(748,"uy",2));    //Uruguay (Eastern Republic of)
    AddEntry(new MccEntry(750,"fk",2));    //Falkland Islands (Malvinas)
}

void MccTable::InitEsmZeroFlagNetworkList()
{
    // ESM flag zero operator
    m_EsmZeroOperator.push_back("46000");
    m_EsmZeroOperator.push_back("46002");
    m_EsmZeroOperator.push_back("46007");
    m_EsmZeroOperator.push_back("45008");
}

void MccTable::InitNotUsePnnOplOperator()
{
    // operators which does not use EF_PNN/EF_OPL
    m_NotUsePnnOplOperator.push_back("334020");
    m_NotUsePnnOplOperator.push_back("722310");
}

void MccTable::AddEntry(MccEntry *entry)
{
    if (entry != NULL) {
        m_mcctable[entry->mcc] = entry;
    }
}

// singleton instance
MccTable MccTable::instance;

MccTable *MccTable::GetInstance()
{
    return &instance;
}

const MccEntry *MccTable::GetEntryForMcc(int mcc)
{
    map<int, MccEntry *> &mcctable = GetInstance()->m_mcctable;
    map<int, MccEntry *>::iterator iter = mcctable.find(mcc);
    if (iter != mcctable.end()) {
        return iter->second;
    }
    return NULL;
}

const char *MccTable::GetCountryCodeForMcc(int mcc)
{
    const MccEntry *entry = GetEntryForMcc(mcc);
    if (entry != NULL) {
        return entry->iso;
    }
    return NULL;
}

const char *MccTable::GetDefaultLanguageForMcc(int mcc)
{
    const MccEntry *entry = GetEntryForMcc(mcc);
    if (entry != NULL) {
        return entry->language;
    }
    return NULL;
}

int MccTable::GetSmallestDigitsMccForMcc(int mcc)
{
    const MccEntry *entry = GetEntryForMcc(mcc);
    if (entry != NULL) {
        return entry->smallestDigitsMcc;
    }
    return 2;
}

int MccTable::GetSmallestDigitsMccForMcc(const string &mcc)
{
    if (mcc.length() >= 3) {
        int value = atoi(mcc.substr(0, 3).c_str());
        return GetSmallestDigitsMccForMcc(value);
    }
    return 2;
}

// MCCMNC_CODES_HAVING_3DIGITS_MNC
// defined in SIMRecord.java in Android telephony framework
// SHOULD be synchronized on every updated
static const char *MCCMNC_CODES_HAVING_3DIGITS_MNC[] = {
        "302370", "302720", "310260",
        "405025", "405026", "405027", "405028", "405029", "405030", "405031", "405032",
        "405033", "405034", "405035", "405036", "405037", "405038", "405039", "405040",
        "405041", "405042", "405043", "405044", "405045", "405046", "405047", "405750",
        "405751", "405752", "405753", "405754", "405755", "405756", "405799", "405800",
        "405801", "405802", "405803", "405804", "405805", "405806", "405807", "405808",
        "405809", "405810", "405811", "405812", "405813", "405814", "405815", "405816",
        "405817", "405818", "405819", "405820", "405821", "405822", "405823", "405824",
        "405825", "405826", "405827", "405828", "405829", "405830", "405831", "405832",
        "405833", "405834", "405835", "405836", "405837", "405838", "405839", "405840",
        "405841", "405842", "405843", "405844", "405845", "405846", "405847", "405848",
        "405849", "405850", "405851", "405852", "405853", "405854", "405855", "405856",
        "405857", "405858", "405859", "405860", "405861", "405862", "405863", "405864",
        "405865", "405866", "405867", "405868", "405869", "405870", "405871", "405872",
        "405873", "405874", "405875", "405876", "405877", "405878", "405879", "405880",
        "405881", "405882", "405883", "405884", "405885", "405886", "405908", "405909",
        "405910", "405911", "405912", "405913", "405914", "405915", "405916", "405917",
        "405918", "405919", "405920", "405921", "405922", "405923", "405924", "405925",
        "405926", "405927", "405928", "405929", "405930", "405931", "405932", "502142",
        "502143", "502145", "502146", "502147", "502148"
};

// MCCMNC_CODES_UNKNOWN
// unknown network which will be reomved in manual search list.
static const char *MCCMNC_CODES_UNKNOWN[] = {
        "72499"
};

// MCCMNC_CODES_USING_SPN_IN_REG_HOME
// MCCMNC code which does use SPN for EONS when it was registered home.
// There is the case in Q (EF_SPN is empty. so AOSP DB value is set to property)
static const char *MCCMNC_CODES_USING_SPN_IN_REG_HOME[] = {
        "72234", "72236", "722341",     // AR PERSONAL
        "722310", "722320", "722330",   // CLARO ARGENTINA
        "72403",                        //TIMBRASIL (TIM)
        "72405",                        // Claro BRA
        "72406", "72410", "72411", "72423",    // VIVO => Vivo
        "26002",                        // T-Mobile Poland
        "52000",                        // TH 3G+ => TRUE-H
};

// MCCMNC_MVNO
// MVNO network list
static const char *MCCMNC_CODES_MVNO[] = {
        // Brazil
        "72417", "72418", "72454",
        // Chile
        "73006", "73008", "73012", "73013", "73019",
        // Colombia
        "732154",
        // Ecuador
        "74003",
};

// The country code to compare between simPlmn and networkPlmn
static const char *MCC_CODES_CHECK_PLMN_MATCHING_FOR_SPN[] = {
        "724", // Brazil
};

// The country code which NITZ has priority than SPN
static const char *MCC_CODES_FOR_NITZ_PRIORITY[] = {
        "716", // Peru
};

// The MCCMNC code which do use SPN for available network search
static const char *MCCMNC_CODES_USING_SPN_IN_PLMN_SRCH[] = {
        "722310", "722320", "722330",   // CLARO ARGENTINA
        "26002",                        // T-Mobile Poland
        "26003",                        //nju (Orange) Poland
};

int MccTable::GetSmallestDigitsMccForImsi(const char *imsi)
{
    if (imsi == NULL) {
        return -1;
    }

    if (strlen(imsi) < 6 || strlen(imsi) > 15) {
        return -1;
    }

    char carrier[7] = { 0, };
    strncpy(carrier, imsi, 6);
    int size = sizeof(MCCMNC_CODES_HAVING_3DIGITS_MNC) / sizeof(MCCMNC_CODES_HAVING_3DIGITS_MNC[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(carrier, MCCMNC_CODES_HAVING_3DIGITS_MNC[i]) == 0) {
            return 3;
        }
    } // end for i ~
    return -1;
}

int MccTable::isUnknowNetwork(const char *plmn)
{
    if (plmn == NULL) {
        return -1;
    }
    if (!(strlen(plmn) == 5 || strlen(plmn) == 6)) {
        return -1;
    }

    char carrier[7] = { 0, };
    strncpy(carrier, plmn, 6);
    int size = sizeof(MCCMNC_CODES_UNKNOWN) / sizeof(MCCMNC_CODES_UNKNOWN[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(carrier, MCCMNC_CODES_UNKNOWN[i]) == 0) {
            return 1;
        }
    }
    return -1;
}

int MccTable::isMvnoNetwork(const char *plmn)
{
    if (plmn == NULL) {
        return -1;
    }
    if (!(strlen(plmn) == 5 || strlen(plmn) == 6)) {
        return -1;
    }

    char carrier[7] = { 0, };
    strncpy(carrier, plmn, 6);
    int size = sizeof(MCCMNC_CODES_MVNO) / sizeof(MCCMNC_CODES_MVNO[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(carrier, MCCMNC_CODES_MVNO[i]) == 0) {
            return 1;
        }
    }
    return -1;
}

bool MccTable::isUsingSpnForOperatorNameInRegHome(const char *plmn)
{
    if (plmn == NULL) {
        return false;
    }

    if (!(strlen(plmn) == 5 || strlen(plmn) == 6)) {
        return false;
    }

    char carrier[7] = { 0, };
    strncpy(carrier, plmn, 6);
    int size = sizeof(MCCMNC_CODES_USING_SPN_IN_REG_HOME) / sizeof(MCCMNC_CODES_USING_SPN_IN_REG_HOME[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(carrier, MCCMNC_CODES_USING_SPN_IN_REG_HOME[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool MccTable::isNeedCheckPlmnMatcingForSpnUsing(const char *plmn)
{
    if (plmn == NULL) {
        return false;
    }

    if (!(strlen(plmn) == 5 || strlen(plmn) == 6)) {
        return false;
    }

    char carrier[4] = { 0, };
    strncpy(carrier, plmn, 3);
    int size = sizeof(MCC_CODES_CHECK_PLMN_MATCHING_FOR_SPN) / sizeof(MCC_CODES_CHECK_PLMN_MATCHING_FOR_SPN[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(carrier, MCC_CODES_CHECK_PLMN_MATCHING_FOR_SPN[i]) == 0) {
            return true;
        }
    }

    return false;
}

bool MccTable::isNitzHasPriority(const char *simPlmn, const char *netPlmn)
{
    bool isMatched = TextUtils::Equals(simPlmn, netPlmn);
    if (isMatched == false) return false;

    if (netPlmn == NULL) {
        return false;
    }

    if (!(strlen(netPlmn) == 5 || strlen(netPlmn) == 6)) {
        return false;
    }

    char carrier[4] = { 0, };
    strncpy(carrier, netPlmn, 3);
    int size = sizeof(MCC_CODES_FOR_NITZ_PRIORITY) / sizeof(MCC_CODES_FOR_NITZ_PRIORITY[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(carrier, MCC_CODES_FOR_NITZ_PRIORITY[i]) == 0) {
            return true;
        }
    }

    return false;
}

bool MccTable::isUsingSpnForAvailablePlmnSrch(const char *plmn)
{
    if (plmn == NULL) {
        return false;
    }

    if (!(strlen(plmn) == 5 || strlen(plmn) == 6)) {
        return false;
    }

    char carrier[7] = { 0, };
    strncpy(carrier, plmn, 6);
    int size = sizeof(MCCMNC_CODES_USING_SPN_IN_PLMN_SRCH) / sizeof(MCCMNC_CODES_USING_SPN_IN_PLMN_SRCH[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(carrier, MCCMNC_CODES_USING_SPN_IN_PLMN_SRCH[i]) == 0) {
            return true;
        }
    }

    return false;
}

int MccTable::GetSmallestDigitsMccForImsi(const string &imsi)
{
    return GetSmallestDigitsMccForImsi(imsi.c_str());
}

bool MccTable::FetchCarrierForImsi(const char *imsi, char *carrier, int size)
{
    if (carrier == NULL) {
        return false;
    }

    if (imsi == NULL) {
        return false;
    }

    if (strlen(imsi) < 6 || strlen(imsi) > 15) {
        return false;
    }

    const int MCC_LENGTH = 3;
    char tmp[7] = {0, };
    int mncLength = GetSmallestDigitsMccForImsi(imsi);
    if (mncLength < 0) {
        strncpy(tmp, imsi, MCC_LENGTH);
        int mcc = atoi(tmp);
        mncLength = GetSmallestDigitsMccForMcc(mcc);

        if (size < MCC_LENGTH + mncLength) {
            return false;
        }
    }
    strncpy(carrier, imsi, MCC_LENGTH + mncLength);
    return true;
}

bool MccTable::IsCarrierUsePnnOplForEons(const char *carrier)
{
    if (TextUtils::IsEmpty(carrier)) {
        return true;
    }

    list<string> &table = GetInstance()->m_NotUsePnnOplOperator;
    list<string>::iterator iter;
    for (iter = table.begin(); iter != table.end(); iter++) {
        if (strcmp((*iter).c_str(), carrier) == 0) {
            return false;
        }
    }

    return true;
}

bool MccTable::IsEsmFlagZeroOperator(const char *carrier)
{
    if (TextUtils::IsEmpty(carrier)) {
        return false;
    }

    char buf[256] = { 0 };
    property_get(PROPERTY_TEST_ESM_ZERO, buf, "");
    if (*buf != 0) {
        if (strcmp(buf, "1") == 0) {
            RilLogV("[%s] %s test ESM zero flag true", TAG, __FUNCTION__);
            return true;
        }
        if (strcmp(buf, "0") == 0) {
            RilLogV("[%s] %s test ESM zero flag false", TAG, __FUNCTION__);
            return false;
        }
    }

    list<string> &table = GetInstance()->m_EsmZeroOperator;
    list<string>::iterator iter;
    for (iter = table.begin(); iter != table.end(); iter++) {
        if (strcmp((*iter).c_str(), carrier) == 0) {
            RilLogV("[%s] %s carrier=%s ESM zero flag true", TAG, __FUNCTION__, carrier);
            return true;
        }
    } // end for iter ~
    RilLogV("[%s] %s carrier=%s ESM zero flag false", TAG, __FUNCTION__, carrier);
    return false;
}

void MccTable::SetEsmFlagZeroOperator(const char *carrier)
{
    if (!TextUtils::IsEmpty(carrier)) {
        list<string> &table = GetInstance()->m_EsmZeroOperator;
        table.push_back(carrier);
    }
}
