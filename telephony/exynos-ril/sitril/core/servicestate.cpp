 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <sstream>
#include "networkutils.h"
#include "rildef.h"
#include "servicestate.h"

ServiceState::ServiceState()
{
    setNullState();
}

void ServiceState::setNullState()
{
    mVoiceRegState = RIL_NOT_REG_AND_NOT_SEARCHING;
    mDataRegState = RIL_NOT_REG_AND_NOT_SEARCHING;
    mOperatorNumeric = "";
    mOperatorAlphaLong = "";
    mOperatorAlphaShort = "";
    mIsManualNetworkSelection = false;
    mVoiceRadioTechnology = RADIO_TECH_UNKNOWN;
    mDataRadioTechnology = RADIO_TECH_UNKNOWN;
    mCssIndicator = -1;
    mNetworkId = -1;
    mSystemId = -1;
    mCdmaRoamingIndicator = -1;
    mCdmaDefaultRoamingIndicator = -1;
    mCdmaEriIconIndex = -1;
    mCdmaEriIconMode = -1;
    mIsEmergencyOnly = false;
    mIsUsingCarrierAggregation = false;
    mChannelNumber = 0;
    mNrStatus = NR_STATUS_NONE;
    mNrFrequencyRange = FREQUENCY_RANGE_UNKNOWN;
    memset(&mLteVopsInfo, 0, sizeof(mLteVopsInfo));
    memset(&mNrIndicators, 0, sizeof(mNrIndicators));
}

ServiceState &ServiceState::operator=(const ServiceState &rhs)
{
    this->mVoiceRegState = rhs.mVoiceRegState;
    this->mDataRegState = rhs.mDataRegState;
    this->mOperatorNumeric = rhs.mOperatorNumeric;
    this->mOperatorAlphaLong = rhs.mOperatorAlphaLong;
    this->mOperatorAlphaShort = rhs.mOperatorAlphaShort;
    this->mIsManualNetworkSelection = rhs.mIsManualNetworkSelection;
    this->mVoiceRadioTechnology = rhs.mVoiceRadioTechnology;
    this->mDataRadioTechnology = rhs.mDataRadioTechnology;
    this->mCssIndicator = rhs.mCssIndicator;
    this->mNetworkId = rhs.mNetworkId;
    this->mSystemId = rhs.mSystemId;
    this->mCdmaRoamingIndicator = rhs.mCdmaRoamingIndicator;
    this->mCdmaDefaultRoamingIndicator = rhs.mCdmaDefaultRoamingIndicator;
    this->mCdmaEriIconIndex = rhs.mCdmaEriIconIndex;
    this->mCdmaEriIconMode = rhs.mCdmaEriIconMode;
    this->mIsEmergencyOnly = rhs.mIsEmergencyOnly;
    this->mIsUsingCarrierAggregation = rhs.mIsUsingCarrierAggregation;
    this->mChannelNumber = rhs.mChannelNumber;
    this->mNrFrequencyRange = rhs.mNrFrequencyRange;
    this->mNrStatus = rhs.mNrStatus;
    this->mLteVopsInfo = rhs.mLteVopsInfo;
    this->mNrIndicators = rhs.mNrIndicators;
    return *this;
}

bool ServiceState::operator==(const ServiceState &rhs)
{
    return (this->mVoiceRegState == rhs.mVoiceRegState) &&
           (this->mDataRegState == rhs.mDataRegState) &&
           (this->mOperatorNumeric == rhs.mOperatorNumeric) &&
           (this->mIsManualNetworkSelection == rhs.mIsManualNetworkSelection) &&
           (this->mVoiceRadioTechnology == rhs.mVoiceRadioTechnology) &&
           (this->mDataRadioTechnology == rhs.mDataRadioTechnology) &&
           (this->mCssIndicator == rhs.mCssIndicator) &&
           (this->mNetworkId == rhs.mNetworkId) &&
           (this->mSystemId == rhs.mSystemId) &&
           (this->mCdmaRoamingIndicator == rhs.mCdmaRoamingIndicator) &&
           (this->mCdmaDefaultRoamingIndicator == rhs.mCdmaDefaultRoamingIndicator) &&
           (this->mCdmaEriIconIndex == rhs.mCdmaEriIconIndex) &&
           (this->mCdmaEriIconMode == rhs.mCdmaEriIconMode) &&
           (this->mIsEmergencyOnly == rhs.mIsEmergencyOnly) &&
           (this->mIsUsingCarrierAggregation == rhs.mIsUsingCarrierAggregation) &&
           (this->mChannelNumber == rhs.mChannelNumber) &&
           (this->mNrFrequencyRange == rhs.mNrFrequencyRange) &&
           (this->mNrStatus == rhs.mNrStatus) &&
           (this->mLteVopsInfo.isVopsSupported == rhs.mLteVopsInfo.isVopsSupported) &&
           (this->mLteVopsInfo.isEmcBearerSupported == rhs.mLteVopsInfo.isEmcBearerSupported) &&
           (this->mNrIndicators.isEndcAvailable == rhs.mNrIndicators.isEndcAvailable) &&
           (this->mNrIndicators.isDcNrRestricted == rhs.mNrIndicators.isDcNrRestricted) &&
           (this->mNrIndicators.isNrAvailable == rhs.mNrIndicators.isNrAvailable);
}

bool ServiceState::operator!=(const ServiceState &rhs)
{
    return !(*this == rhs);
}

const string ServiceState::toString() const
{
    stringstream ss;
    ss << "{";
    ss << "mVoiceRegState=";
    ss << NetworkUtils::getRegStateString(mVoiceRegState);
    ss << ",mDataRegState=";
    ss << NetworkUtils::getRegStateString(mDataRegState);
    ss << ",mOperatorNumeric=";
    ss << mOperatorNumeric;
    ss << ",mOperatorAlphaLong=";
    ss << mOperatorAlphaLong;
    ss << ",mOperatorAlphaShort=";
    ss << mOperatorAlphaShort;
    ss << ",mIsManualNetworkSelection=";
    ss << mIsManualNetworkSelection;
    ss << ",mVoiceRadioTechnology=";
    ss << NetworkUtils::getRadioTechnologyString(mVoiceRadioTechnology);
    ss << ",mDataRadioTechnology=";
    ss << NetworkUtils::getRadioTechnologyString(mDataRadioTechnology);
    ss << (mIsUsingCarrierAggregation ? "(CA)" : "");
    ss << ",mIsUsingCarrierAggregation=";
    ss << mIsUsingCarrierAggregation;
    ss << ",mIsEmergencyOnly=";
    ss << mIsEmergencyOnly;
    ss << ",mChannelNumber=";
    ss << mChannelNumber;
    ss << ",mNrFrequencyRange=";
    ss << mNrFrequencyRange;
    ss << ",mNrStatus=";
    ss << mNrStatus;
    ss << ",mLteVopsInfo.isVopsSupported=";
    ss << mLteVopsInfo.isVopsSupported;
    ss << ",mLteVopsInfo.isEmcBearerSupported=";
    ss << mLteVopsInfo.isEmcBearerSupported;
    ss << ",mNrIndicators.isEndcAvailable=";
    ss << mNrIndicators.isEndcAvailable;
    ss << ",mNrIndicators.isDcNrRestricted=";
    ss << mNrIndicators.isDcNrRestricted;
    ss << ",mNrIndicators.isNrAvailable=";
    ss << mNrIndicators.isNrAvailable;
    ss << "}";
    return ss.str();
}

void ServiceState::setVoiceRegState(int voiceRegState)
{
    switch (voiceRegState) {
    case RIL_NOT_REG_AND_NOT_SEARCHING:
    case RIL_REG_HOME:
    case RIL_NOT_REG_AND_SEARCHING:
    case RIL_REG_DENIED:
    case RIL_UNKNOWN:
    case RIL_REG_ROAMING:
    case RIL_NOT_REG_AND_EMERGENCY_AVAILABLE_AND_NOT_SEARCHING:
    case RIL_NOT_REG_AND_EMERGENCY_AVAILABLE_AND_SEARCHING:
    case RIL_REG_DENIED_AND_EMERGENCY_AVAILABLE:
    case RIL_UNKNOWN_AND_EMERGENCY_AVAILABLE:
        mVoiceRegState = voiceRegState;
        break;
    default:
        mVoiceRegState = RIL_NOT_REG_AND_NOT_SEARCHING;
        break;
    }
}

void ServiceState::setDataRegState(int dataRegState)
{
    switch (dataRegState) {
    case RIL_NOT_REG_AND_NOT_SEARCHING:
    case RIL_REG_HOME:
    case RIL_NOT_REG_AND_SEARCHING:
    case RIL_REG_DENIED:
    case RIL_UNKNOWN:
    case RIL_REG_ROAMING:
    case RIL_NOT_REG_AND_EMERGENCY_AVAILABLE_AND_NOT_SEARCHING:
    case RIL_NOT_REG_AND_EMERGENCY_AVAILABLE_AND_SEARCHING:
    case RIL_REG_DENIED_AND_EMERGENCY_AVAILABLE:
    case RIL_UNKNOWN_AND_EMERGENCY_AVAILABLE:
        mDataRegState = dataRegState;
        break;
    default:
        mDataRegState = RIL_NOT_REG_AND_NOT_SEARCHING;
        break;
    }
}

void ServiceState::setVoiceRadioTechnology(int rt)
{
    const int RADIO_TECH_MAX = RADIO_TECH_NR;
    if (rt < 0 || rt > RADIO_TECH_MAX) {
        mVoiceRadioTechnology = RADIO_TECH_UNKNOWN;
    }
    else {
        mVoiceRadioTechnology = rt;
    }
}

void ServiceState::setDataRadioTechnology(int rt)
{
    const int RADIO_TECH_MAX = RADIO_TECH_NR;
    if (rt < 0 || rt > RADIO_TECH_MAX) {
        mDataRadioTechnology = RADIO_TECH_UNKNOWN;
    }

    mDataRadioTechnology = rt;
    if (mDataRadioTechnology == RADIO_TECH_LTE_CA) {
        mIsUsingCarrierAggregation = true;
        mDataRadioTechnology = RADIO_TECH_LTE;
    }
    else {
        mIsUsingCarrierAggregation = false;
    }
}

void ServiceState::setOperatorName(const string &longName, const string &shortName)
{
    this->mOperatorAlphaLong = longName;
    this->mOperatorAlphaShort = shortName;
}


void ServiceState::updateNrStatus(bool isEndcAvailable, bool isDcNrRestricted, bool isNrAvailable)
{
    mNrIndicators.isEndcAvailable = isEndcAvailable;
    mNrIndicators.isDcNrRestricted = isDcNrRestricted;
    mNrIndicators.isNrAvailable = isNrAvailable;

    mNrStatus = NR_STATUS_NONE;
    if (isEndcAvailable) {
        if (!isDcNrRestricted && isNrAvailable) {
            mNrStatus = NR_STATUS_NOT_RESTRICTED;
        }
        else {
            mNrStatus = NR_STATUS_RESTRICTED;
        }
    }
}
