package com.samsung.slsi.libovsecc;

import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.List;

import java.util.logging.Logger;
import java.util.logging.Level;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

public class EccListLoader {

    private static final Logger logger = Logger.getLogger(EccListLoader.class.getName());

    private static final String ENTITY_MCCMNC = "MccmncValue";
    private static final String ENTITY_ATTR_MCCMNC = "Mccmnc";

    // If there's no specific MCCMNC, Retail entity is used.
    // Retail entity is located at the last.
    private static final String ENTITY_MCCMNC_VALUE_RETAIL = "#R";

    private static final String ENTITY_NAME = "EccEntry";
    private static final String ENTITY_ATTR_ECC = "Ecc";
    private static final String ENTITY_ATTR_CATEGORY = "Category";
    private static final String ENTITY_ATTR_CONDITION = "Condition";

    private static final int ENTITY_ATTR_MCCMNC_INDEX       = 0;

    private static final int ENTITY_ATTR_ECC_INDEX       = 0;
    private static final int ENTITY_ATTR_CATEGORY_INDEX  = 1;
    private static final int ENTITY_ATTR_CONDITION_INDEX = 2;

    private static class EmergencyEntry {
        String ecc;
        int category;
        int condition;

        EmergencyEntry(String _ecc, int _category, int _condition) {
            ecc = _ecc;
            category = _category;
            condition = _condition;
        }
    }

    private static List<EmergencyEntry> mEmergencyEntries = new ArrayList<EmergencyEntry>();
    private static String mIso = "";
    private static String mMccmnc = "";
    private static EccListLoader sInstance;

    public EccListLoader() {
    }

    public static synchronized EccListLoader getInstance(String iso, String mccmnc) {
        if (sInstance == null) {
            sInstance = new EccListLoader();
        }
        if (sInstance.mIso != null
                && iso != null
                && (!sInstance.mIso.equals(iso) || !sInstance.mMccmnc.equals(mccmnc))) {
            sInstance.mIso = iso;
            sInstance.mMccmnc = mccmnc;
            sInstance.readEccListFromXml(iso, mccmnc);
        }
        return sInstance;
     }

   /**
    * Returns true if the number exactly matches an emergency service number.
    *
    * @param number  the phone number
    * @return if the number is matched an emergency services number in the given list of emergency entries.
    */
    public static boolean isEmergencyNumber(String number) {
        logger.log(Level.WARNING, "isEmergencyNumber number=" + number);

        for (EmergencyEntry entry : mEmergencyEntries){
            if (entry.ecc.equals(number)) {
                return true;
            }
        }
        return false;
    }

   /**
    * Returns category if the number exactly matches an emergency service number.
    *
    * @param number  the phone number
    * @return category if the number is matched an emergency services number.
    */
    public static int getCategory(String number) {
        logger.log(Level.WARNING, "getCategory number=" + number);
        for (EmergencyEntry entry : mEmergencyEntries){
            if (entry.ecc.equals(number)) {
                return entry.category;
            }
        }
        return 0;
    }

   /**
    * Returns condition if the number exactly matches an emergency service number.
    *
    * @param number  the phone number
    * @return condition if the number is matched an emergency services number.
    */
    public static int getCondition(String number) {
        logger.log(Level.WARNING, "getCondition number=" + number);
        for (EmergencyEntry entry : mEmergencyEntries){
            if (entry.ecc.equals(number)) {
                return entry.condition;
            }
        }
        return 0;
    }

    private static void readEccListFromXml(String iso, String mccmnc) {
        String fileName = "/com/samsung/slsi/libovsecc/data/EccTable_" + iso + ".xml";
        logger.log(Level.WARNING, "readEccListFromXml fileName: " + fileName);
        if (EccListLoader.class.getResourceAsStream(fileName) == null) {
            logger.log(Level.WARNING, "No xml file, uses the DEFAULT.xml");
            fileName = "/com/samsung/slsi/libovsecc/data/EccTable_DEFAULT.xml";
        } else {
            logger.log(Level.WARNING, "Found xml file");
        }
        mEmergencyEntries = new ArrayList<EmergencyEntry>();
        try {
            XmlPullParserFactory parserFactory = XmlPullParserFactory.newInstance();
            XmlPullParser parser = parserFactory.newPullParser();
            parser.setInput(EccListLoader.class.getResourceAsStream(fileName), "utf-8");

            int event;
            String ecc;
            int category, condition;
            while (((event = parser.next()) != XmlPullParser.END_DOCUMENT)) {
                if (event == XmlPullParser.START_TAG && ENTITY_MCCMNC.equals(parser.getName())) {
                    if (ENTITY_ATTR_MCCMNC.equals(parser.getAttributeName(ENTITY_ATTR_MCCMNC_INDEX))
                            && (mccmnc.equals(parser.getAttributeValue(ENTITY_ATTR_MCCMNC_INDEX))
                                || ENTITY_MCCMNC_VALUE_RETAIL.equals(parser.getAttributeValue(ENTITY_ATTR_MCCMNC_INDEX)))) {
                        event = parser.next();
                        while (event != XmlPullParser.END_TAG || !ENTITY_MCCMNC.equals(parser.getName())) {
                            if (event == XmlPullParser.START_TAG && ENTITY_NAME.equals(parser.getName())) {
                                ecc = null;
                                category = 0;
                                condition = 1;
                                if (ENTITY_ATTR_ECC.equals(parser.getAttributeName(ENTITY_ATTR_ECC_INDEX))) {
                                    ecc =  parser.getAttributeValue(ENTITY_ATTR_ECC_INDEX);
                                    //logger.log(Level.WARNING, "readEccListFromXml ECC: " + ecc);
                                }
                                if (ENTITY_ATTR_CATEGORY.equals(parser.getAttributeName(ENTITY_ATTR_CATEGORY_INDEX))) {
                                    category = Integer.parseInt(parser.getAttributeValue(ENTITY_ATTR_CATEGORY_INDEX));
                                    //logger.log(Level.WARNING, "readEccListFromXml Category: " + category);
                                }
                                if (ENTITY_ATTR_CONDITION.equals(parser.getAttributeName(ENTITY_ATTR_CONDITION_INDEX))) {
                                    condition = Integer.parseInt(parser.getAttributeValue(ENTITY_ATTR_CONDITION_INDEX));
                                    //logger.log(Level.WARNING, "readEccListFromXml Condition: " + condition);
                                }

                                if (ecc != null) {
                                    mEmergencyEntries.add(new EmergencyEntry(ecc, category, condition));
                                }
                            }
                            event = parser.next();
                        }
                        break;
                    }
                }
            }
            printAllEntries();
        } catch (IOException | XmlPullParserException e) {
            logger.log(Level.WARNING, "readEccListFromXml IOException | XmlPullParserException: " + e);
        } catch (Exception e) {
            logger.log(Level.WARNING, "readEccListFromXml Exception: " + e);
        }
    }

    private static void printAllEntries() {
        for (EmergencyEntry entry : mEmergencyEntries){
            logger.log(Level.WARNING, "printAllEntries ecc=" + entry.ecc +
                                      " category=" + entry.category + " condition= " + entry.condition);
        }
    }
}
