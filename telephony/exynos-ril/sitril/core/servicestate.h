 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include <string>
using namespace std;

class ServiceState
{
private:
    int mVoiceRegState;
    int mDataRegState;
    string mOperatorNumeric;
    string mOperatorAlphaLong;
    string mOperatorAlphaShort;
    bool mIsManualNetworkSelection;
    int mVoiceRadioTechnology;
    int mDataRadioTechnology;
    int mCssIndicator;
    int mNetworkId;
    int mSystemId;
    int mCdmaRoamingIndicator;
    int mCdmaDefaultRoamingIndicator;
    int mCdmaEriIconIndex;
    int mCdmaEriIconMode;
    int mIsEmergencyOnly;
    int mIsUsingCarrierAggregation;
    int mChannelNumber;
    RIL_LteVopsInfo mLteVopsInfo;
    RIL_NrIndicators mNrIndicators;
    int mNrStatus;
    int mNrFrequencyRange;
public:
    ServiceState();
    void setNullState();
    ServiceState &operator=(const ServiceState &rhs);
    bool operator==(const ServiceState &rhs);
    bool operator!=(const ServiceState &rhs);
    const string toString() const;

    int getVoiceRegState() { return mVoiceRegState; }
    int getDataRegState() { return mDataRegState; }
    const string &getOperatorNumeric() const { return mOperatorNumeric; }
    const string &getOperatorAlphaLong() const { return mOperatorAlphaLong; }
    const string &getOperatorAlphaShort() const { return mOperatorAlphaShort; }
    bool getIsManualSelection() { return mIsManualNetworkSelection; }
    int getVoiceRadioTechnology() { return mVoiceRadioTechnology; }
    int getDataRadioTechnology() { return mDataRadioTechnology; }
    bool isEmergencyOnly() { return mIsEmergencyOnly; }
    bool isUsingCarrierAggregation() { return mIsUsingCarrierAggregation; }
    int getChannelNumber() { return mChannelNumber; }
    bool getLteVopsSupport() { return mLteVopsInfo.isVopsSupported; }
    bool getLteEmcBearerSupport() { return mLteVopsInfo.isEmcBearerSupported; }
    int getNrStatus() { return mNrStatus; }
    bool isEndcAvailable() { return mNrIndicators.isEndcAvailable; }
    bool isDcNrRestricted() { return mNrIndicators.isDcNrRestricted; }
    bool isNrAvailable() { return mNrIndicators.isNrAvailable; }
    void setVoiceRegState(int voiceRegState);
    void setDataRegState(int dataRegState);
    void setOperatorNumeric(string numeric) { mOperatorNumeric = numeric; }
    void setIsManualSelection(bool isManual) { mIsManualNetworkSelection = isManual; }
    void setVoiceRadioTechnology(int rt);
    void setDataRadioTechnology(int rt);
    void setOperatorName(const string &longName, const string &shortName);
    void setEmergencyOnly(bool emergencyOnly) { mIsEmergencyOnly = emergencyOnly; }
    void setChannelNumber(int channelNumber) { mChannelNumber = channelNumber; }
    void setLteVopsSupport(bool vopsSupport) { mLteVopsInfo.isVopsSupported = vopsSupport; }
    void setLteEmcBearerSupport(bool emcBearerSupport) { mLteVopsInfo.isEmcBearerSupported = emcBearerSupport; }
    void updateNrStatus(bool isEndcAvailable, bool isDcNrRestricted, bool isNrAvailable);
    void setNrStatus(int nrStatus) { mNrStatus = nrStatus; }
};
