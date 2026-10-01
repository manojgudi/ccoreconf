#ifndef __TEST_SCHC_IMPL_H__
#define __TEST_SCHC_IMPL_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <coreconfTypes.h>
#include <coreconfManipulation.h>
#include <hashmap.h>
#define read_ietfSchc_schc read_60095
#define write_ietfSchc_schc write_60095
CoreconfValueT* read_ietfSchc_schc(void);
int write_ietfSchc_schc(CoreconfValueT* value);
#define read_schc_rule read_60096
#define write_schc_rule write_60096
CoreconfValueT* read_schc_rule(uint64_t rule_ruleIdValue, uint64_t rule_ruleIdLength);
int write_schc_rule(uint64_t rule_ruleIdValue, uint64_t rule_ruleIdLength, CoreconfValueT* value);
#define read_rule_ackBehavior read_60097
#define write_rule_ackBehavior write_60097
uint64_t read_rule_ackBehavior(void);
int write_rule_ackBehavior(uint64_t value);
#define read_rule_direction read_60098
#define write_rule_direction write_60098
uint64_t read_rule_direction(void);
int write_rule_direction(uint64_t value);
#define read_rule_dtagSize read_60099
#define write_rule_dtagSize write_60099
uint64_t read_rule_dtagSize(void);
int write_rule_dtagSize(uint64_t value);
#define read_rule_entry read_60100
#define write_rule_entry write_60100
CoreconfValueT* read_rule_entry(uint64_t entry_fieldId, uint64_t entry_fieldPosition, uint64_t entry_directionIndicator);
int write_rule_entry(uint64_t entry_fieldId, uint64_t entry_fieldPosition, uint64_t entry_directionIndicator, CoreconfValueT* value);
#define read_entry_compDecompAction read_60101
#define write_entry_compDecompAction write_60101
uint64_t read_entry_compDecompAction(void);
int write_entry_compDecompAction(uint64_t value);
#define read_entry_compDecompActionValue read_60102
#define write_entry_compDecompActionValue write_60102
CoreconfValueT* read_entry_compDecompActionValue(uint64_t compDecompActionValue_index);
int write_entry_compDecompActionValue(uint64_t compDecompActionValue_index, CoreconfValueT* value);
#define read_compDecompActionValue_value read_60104
#define write_compDecompActionValue_value write_60104
bool read_compDecompActionValue_value(void);
int write_compDecompActionValue_value(bool value);
#define read_entry_fieldLength read_60107
#define write_entry_fieldLength write_60107
CoreconfValueT* read_entry_fieldLength(void);
int write_entry_fieldLength(CoreconfValueT* value);
#define read_entry_matchingOperator read_60109
#define write_entry_matchingOperator write_60109
uint64_t read_entry_matchingOperator(void);
int write_entry_matchingOperator(uint64_t value);
#define read_entry_matchingOperatorValue read_60110
#define write_entry_matchingOperatorValue write_60110
CoreconfValueT* read_entry_matchingOperatorValue(uint64_t matchingOperatorValue_index);
int write_entry_matchingOperatorValue(uint64_t matchingOperatorValue_index, CoreconfValueT* value);
#define read_matchingOperatorValue_value read_60112
#define write_matchingOperatorValue_value write_60112
bool read_matchingOperatorValue_value(void);
int write_matchingOperatorValue_value(bool value);
#define read_entry_targetValue read_60113
#define write_entry_targetValue write_60113
CoreconfValueT* read_entry_targetValue(uint64_t targetValue_index);
int write_entry_targetValue(uint64_t targetValue_index, CoreconfValueT* value);
#define read_targetValue_value read_60115
#define write_targetValue_value write_60115
bool read_targetValue_value(void);
int write_targetValue_value(bool value);
#define read_rule_fcnSize read_60116
#define write_rule_fcnSize write_60116
uint64_t read_rule_fcnSize(void);
int write_rule_fcnSize(uint64_t value);
#define read_rule_fragmentationMode read_60117
#define write_rule_fragmentationMode write_60117
uint64_t read_rule_fragmentationMode(void);
int write_rule_fragmentationMode(uint64_t value);
#define read_rule_inactivityTimer read_60118
#define write_rule_inactivityTimer write_60118
CoreconfValueT* read_rule_inactivityTimer(void);
int write_rule_inactivityTimer(CoreconfValueT* value);
#define read_inactivityTimer_ticksDuration read_60119
#define write_inactivityTimer_ticksDuration write_60119
uint64_t read_inactivityTimer_ticksDuration(void);
int write_inactivityTimer_ticksDuration(uint64_t value);
#define read_inactivityTimer_ticksNumbers read_60120
#define write_inactivityTimer_ticksNumbers write_60120
uint64_t read_inactivityTimer_ticksNumbers(void);
int write_inactivityTimer_ticksNumbers(uint64_t value);
#define read_rule_l2WordSize read_60121
#define write_rule_l2WordSize write_60121
uint64_t read_rule_l2WordSize(void);
int write_rule_l2WordSize(uint64_t value);
#define read_rule_maxAckRequests read_60122
#define write_rule_maxAckRequests write_60122
uint64_t read_rule_maxAckRequests(void);
int write_rule_maxAckRequests(uint64_t value);
#define read_rule_maxInterleavedFrames read_60123
#define write_rule_maxInterleavedFrames write_60123
uint64_t read_rule_maxInterleavedFrames(void);
int write_rule_maxInterleavedFrames(uint64_t value);
#define read_rule_maximumPacketSize read_60124
#define write_rule_maximumPacketSize write_60124
uint64_t read_rule_maximumPacketSize(void);
int write_rule_maximumPacketSize(uint64_t value);
#define read_rule_rcsAlgorithm read_60125
#define write_rule_rcsAlgorithm write_60125
uint64_t read_rule_rcsAlgorithm(void);
int write_rule_rcsAlgorithm(uint64_t value);
#define read_rule_retransmissionTimer read_60126
#define write_rule_retransmissionTimer write_60126
CoreconfValueT* read_rule_retransmissionTimer(void);
int write_rule_retransmissionTimer(CoreconfValueT* value);
#define read_retransmissionTimer_ticksDuration read_60127
#define write_retransmissionTimer_ticksDuration write_60127
uint64_t read_retransmissionTimer_ticksDuration(void);
int write_retransmissionTimer_ticksDuration(uint64_t value);
#define read_retransmissionTimer_ticksNumbers read_60128
#define write_retransmissionTimer_ticksNumbers write_60128
uint64_t read_retransmissionTimer_ticksNumbers(void);
int write_retransmissionTimer_ticksNumbers(uint64_t value);
#define read_rule_ruleNature read_60131
#define write_rule_ruleNature write_60131
uint64_t read_rule_ruleNature(void);
int write_rule_ruleNature(uint64_t value);
#define read_rule_tileInAll1 read_60132
#define write_rule_tileInAll1 write_60132
uint64_t read_rule_tileInAll1(void);
int write_rule_tileInAll1(uint64_t value);
#define read_rule_tileSize read_60133
#define write_rule_tileSize write_60133
uint64_t read_rule_tileSize(void);
int write_rule_tileSize(uint64_t value);
#define read_rule_wSize read_60134
#define write_rule_wSize write_60134
uint64_t read_rule_wSize(void);
int write_rule_wSize(uint64_t value);
#define read_rule_windowSize read_60135
#define write_rule_windowSize write_60135
uint64_t read_rule_windowSize(void);
int write_rule_windowSize(uint64_t value);
#define read_ietfSchcOam_proxyBehavior read_2000010
#define write_ietfSchcOam_proxyBehavior write_2000010
uint64_t read_ietfSchcOam_proxyBehavior(void);
int write_ietfSchcOam_proxyBehavior(uint64_t value);


// User-facing read/write function prototypes
// Implement these functions in your code

#endif
