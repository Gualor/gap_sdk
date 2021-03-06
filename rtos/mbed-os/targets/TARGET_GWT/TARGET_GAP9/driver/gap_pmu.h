/*
 * Copyright (c) 2018, GreenWaves Technologies, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of GreenWaves Technologies, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _GAP_PMU_H_
#define _GAP_PMU_H_

#include "cmsis.h"
#include "gap_itc.h"

/*!
 * @addtogroup pmu
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define    PMU_MAX_LV_FREQUENCY       150000000
#define    PMU_MAX_NV_FREQUENCY       250000000

/* Voltage regulator and power states */
#define    PMU_DCDC_OPER_POINTS       4

#define    PMU_DCDC_DEFAULT_NV        1200
#define    PMU_DCDC_DEFAULT_MV        1200
#define    PMU_DCDC_DEFAULT_LV        1000
#define    PMU_DCDC_DEFAULT_RET       800

#define    PMU_DCDC_RANGE             5
#define    PMU_DCDC_RANGE_MASK        0x1F

#define    PMU_DCDC_BASE_VALUE        550
#define    PMU_DCDC_STEP              50

/* PMU error type */
#define    PMU_ERROR_NO_ERROR                      (0)
#define    PMU_ERROR_FREQ_TOO_HIGH                 (0x1<<0)
#define    PMU_ERROR_NEED_STATE_CHANGE             (0x1<<1)
#define    PMU_ERROR_VDD_CHANGE_TOO_HIGH           (0x1<<2)
#define    PMU_ERROR_SOC_STATE_CHANGE_FAILED       (0x1<<3)
#define    PMU_ERROR_CLUSTER_STATE_CHANGE_FAILED   (0x1<<4)

#define    PMU_MAX_DCDC_VARIATION   (int) (0.1*32767)

/* PMU State:
   State:          SOC_LP/HP, SOC_CLUSTER_LP/HP, RETENTIVE, DEEP_SLEEP.
   Bits[2:1]:      Regulator State, Bit[0], Cluster On/Off
   FllClusterDown  Cluster FLL stop mode: FLL_FAST_WAKEUP, FLL_RETENTIVE or FLL_SHUTDOWN
   DCDC_Settings[]
   REGU_NV         Nominal,   BaseValue + Incr*ValueNv             BaseValue and Incr are constants (550, 50)
   REGU_LV         Low,       BaseValue + Incr*ValueLv             BaseValue and Incr are constants (550, 50)
   REGU_RET        Retentive, BaseValue + Incr*ValueRet            BaseValue and Incr are constants (550, 50)
   REGU_OFF        0
   Fll SoC:         Frequency:      Value in Hz
   Fll Cluster:     Frequency:      Value in Hz
*/
typedef enum _pmu_fll_cluster_down {
  uPMU_FLL_FAST_WAKEUP=0,    /* Fll is kept active and ready to be used */
  uPMU_FLL_RETENTIVE=1,      /* Fll is stopped and it's state is kept in retention */
  uPMU_FLL_SHUTDOWN=2        /* Fll is stopped and has to be re initialized when restarting */
} pmu_fll_cluster_down_t;

typedef struct _pmu_state {
  uint8_t   State;
  uint8_t   FllClusterDown;
  uint8_t   DCDC_Settings[4];
  uint32_t  Frequency[3];
} pmu_state_t;

extern pmu_state_t   PMU_State;

#ifdef FEATURE_CLUSTER
extern uint32_t pmu_events_status;
#endif

/* Maestro internal events */
#define    PMU_MAESTRO_EVENT_ICU_OK              (1<<0)
#define    PMU_MAESTRO_EVENT_ICU_DELAYED         (1<<1)
#define    PMU_MAESTRO_EVENT_MODE_CHANGED        (1<<2)
#define    PMU_MAESTRO_EVENT_PICL_OK             (1<<3)
#define    PMU_MAESTRO_EVENT_SCU_OK              (1<<4)

#define    PMU_EVENT_CLUSTER_ON_OFF     (1 << 0) /* GAP_SOC_EVENT_CLUSTER_ON_OFF */
#define    PMU_EVENT_MSP                (1 << 1) /* GAP_SOC_EVENT_MSP) */
#define    PMU_EVENT_ICU_MODE_CHANGED   (1 << 2) /* GAP_SOC_EVENT_ICU_MODE_CHANGED) */
#define    PMU_EVENT_ICU_OK             (1 << 3) /* GAP_SOC_EVENT_ICU_OK) */
#define    PMU_EVENT_CLUSTER_CLOCK_GATE (1 << 4) /* GAP_SOC_EVENT_ICU_DELAYED) */
#define    PMU_EVENT_PICL_OK            (1 << 5) /* GAP_SOC_EVENT_PICL_OK) */
#define    PMU_EVENT_SCU_OK             (1 << 6) /* GAP_SOC_EVENT_SCU_OK) */
#define    PMU_EVENT_SCU_PENDING        (1 << 7) /* GAP_SOC_EVENT_PMU_NB_EVENTS) */

typedef enum _pmu_scu_seq {
  uPMU_SCU_DEEP_SLEEP     = 0,
  uPMU_SCU_RETENTIVE      = 1,
  uPMU_SCU_SOC_HP         = 2,
  uPMU_SCU_SOC_LP         = 3,
  uPMU_SCU_SOC_CLUSTER_HP = 4,
  uPMU_SCU_SOC_CLUSTER_LP = 5,
} pmu_scu_seq_t;

typedef enum _pmu_wakeup_state {    /* Enum encoding follows the definition of the boot_type field of PMU_RetentionStateT */
  uPMU_COLD_BOOT       = 0, /* SoC cold boot, from Flash usually */
  uPMU_DEEP_SLEEP_BOOT = 1, /* Reboot from deep sleep state, state has been lost, somehow equivalent to COLD_BOOT */
  uPMU_RETENTIVE_BOOT  = 2, /* Reboot from Retentive state, state has been retained, should bypass flash reload */
} pmu_wakeup_state_t;

typedef enum _pmu_boot_mode {
  uPMU_BOOT_FROM_ROM=0,
  uPMU_BOOT_FROM_L2=1,
} pmu_boot_mode_t;

typedef enum _pmu_wakeup_mode {
  uPMU_EXT_WAKEUP_RAISING_EDGE = 0,
  uPMU_EXT_WAKEUP_FALLING_EDGE = 1,
  uPMU_EXT_WAKEUP_HIGH = 2,
  uPMU_EXT_WAKEUP_LOW = 3,
} pmu_wakeup_mode_t;

typedef enum pmu_status {
  uPMU_CHANGE_OK = 0,
  uPMU_CHANGE_ERROR = 1
} pmu_status_t;

typedef enum _pmu_dcdc_hw_operatingpoint {
  uPMU_DCDC_Nominal   = 0,
  uPMU_DCDC_Medium    = 1,
  uPMU_DCDC_Low       = 2,
  uPMU_DCDC_Retentive = 3,
} pmu_dcdc_hw_operatingpoint_t;


typedef enum _pmu_cluster_state {
  uPMU_CLUSTER_OFF = 0,
  uPMU_CLUSTER_ON  = 1
} pmu_cluster_state_t;

typedef enum _pmu_regulator_state {
  uPMU_REGU_NV  = 0,
  uPMU_REGU_LV  = 1,
  uPMU_REGU_RET = 2,
  uPMU_REGU_OFF = 3
} pmu_regulator_state_t;

typedef enum _pmu_system_state {
  uPMU_SOC_CLUSTER_HP = ((uPMU_REGU_NV<<1)  | uPMU_CLUSTER_ON),  /* 001 = 1 */
  uPMU_SOC_CLUSTER_LP = ((uPMU_REGU_LV<<1)  | uPMU_CLUSTER_ON),  /* 011 = 3 */
  uPMU_SOC_HP         = ((uPMU_REGU_NV<<1)  | uPMU_CLUSTER_OFF), /* 000 = 0 */
  uPMU_SOC_LP         = ((uPMU_REGU_LV<<1)  | uPMU_CLUSTER_OFF), /* 010 = 2 */
  uPMU_RETENTIVE      = ((uPMU_REGU_RET<<1) | uPMU_CLUSTER_OFF), /* 100 = 4 */
  uPMU_DEEP_SLEEP     = ((uPMU_REGU_OFF<<1) | uPMU_CLUSTER_OFF)  /* 110 = 6 */
} pmu_system_state_t;

typedef enum _pmu_fll_type {
  uPMU_FLL_SOC=0,
  uPMU_FLL_CLUSTER=1,
} pmu_fll_type_t;


#define PMU_CLUSTER_STATE_MASK              (0x1U)
#define PMU_CLUSTER_STATE_SHIFT             (0U)
#define PMU_CLUSTER_STATE(x)                (((uint32_t)(((uint32_t)(x)) /* << PMU_CLUSTER_STATE_SHIFT)*/)) & PMU_CLUSTER_STATE_MASK)
#define READ_PMU_CLUSTER_STATE(x)           (((uint32_t)(((uint32_t)(x)) & PMU_CLUSTER_STATE_MASK)) /*>> PMU_CLUSTER_STATE_SHIFT*/)

#define PMU_REGULATOR_STATE_MASK            (0x6U)
#define PMU_REGULATOR_STATE_SHIFT           (1U)
#define PMU_REGULATOR_STATE(x)              (((uint32_t)(((uint32_t)(x)) << PMU_REGULATOR_STATE_SHIFT))) & PMU_REGULATOR_STATE_MASK)
#define READ_PMU_REGULATOR_STATE(x)         (((uint32_t)(((uint32_t)(x)) & PMU_REGULATOR_STATE_MASK)) >> PMU_REGULATOR_STATE_SHIFT)

#define PMU_POWER_SYSTEM_STATE(ReguState, ClusterState) ((pmu_system_state_t) ((ReguState << PMU_REGULATOR_STATE_SHIFT) | ClusterState))

#define mV_TO_DCDC(mV)    ((uint32_t) (((mV) - PMU_DCDC_BASE_VALUE) / PMU_DCDC_STEP))
#define DCDC_TO_mV(Dc)    ((uint32_t) ((Dc) * PMU_DCDC_STEP + PMU_DCDC_BASE_VALUE))

/*******************************************************************************
 * APIs
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Initializes the PMU.
 *
 * This function
 *
 * @note .
 */
void PMU_Init();

/*!
 * @brief Set PUM voltage and check frequency at the same time .
 *
 * This function
 *
 * @note .
 */
int PMU_SetVoltage(uint32_t voltage, uint32_t frequency_check);

/*!
 * @brief De-Initializes and shut down the PMU.
 *
 * This function
 *
 * @note .
 */
void PMU_DeInit(int retentive, pmu_system_state_t wakeup_state);

/*!
 * @brief MRAM power on management.
 *
 * This function power on the MRAM domain
 *
 * @note .
 */
void PMU_MramPowerOn();

/*!
 * @brief MRAM power off management.
 *
 * This function power off the MRAM domain
 *
 * @note .
 */
void PMU_MramPowerOff();

/*!
 * @brief CSI2 power on management.
 *
 * This function power on the CSI2 domain
 *
 * @note .
 */
void PMU_CSI2PowerOn();

/*!
 * @brief CSI2 power off management.
 *
 * This function power off the CSI2 domain
 *
 * @note .
 */
void PMU_CSI2PowerOff();

/*!
 * @brief Smart Wake power on management.
 *
 * This function power on the Smart Wake domain
 *
 * @note .
 */
void PMU_SmartWakeOn();

/*!
 * @brief Smart Wake power off management.
 *
 * This function power off the Smart Wake domain
 *
 * @note .
 */
void PMU_SmartWakeOff();

#ifdef FEATURE_CLUSTER
/*!
 * @brief Check cluster power is on.
 *
 * This function check cluster domain power on
 *
 * @note .
 */
static inline int PMU_ClusterIsOn() {
    return (READ_PMU_CLUSTER_STATE(PMU_State.State) == uPMU_CLUSTER_ON);
}

/*!
 * @brief Check cluster power is off.
 *
 * This function check cluster domain power off
 *
 * @note .
 */
static inline int PMU_ClusterIsOff() {
    return (READ_PMU_CLUSTER_STATE(PMU_State.State) == uPMU_CLUSTER_OFF);
}

/*!
 * @brief Cluster power on management.
 *
 * This function power on the cluster domain
 *
 * @note .
 */
void PMU_ClusterPowerOn();

/*!
 * @brief Cluster power off management.
 *
 * This function power off the cluster domain
 *
 * @note .
 */
void PMU_ClusterPowerOff();
#endif


/* @} */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/* @} */

#endif /*_GAP_PMU_H_*/
