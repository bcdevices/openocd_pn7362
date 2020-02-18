/*
 *                  Copyright (c), NXP Semiconductors
 *
 *                     (C)NXP Semiconductors
 *       All rights are reserved. Reproduction in whole or in part is
 *      prohibited without the written consent of the copyright owner.
 *  NXP reserves the right to make changes without notice at any time.
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 *particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 *                          arising from its use.
 */
/** \file
 * Status return values.
 * $Author: Purnank G (ing05193) $
 * $Revision: 18101 $
 * $Date: 2016-03-07 18:20:50 +0530 (Mon, 07 Mar 2016) $
 *
 * History:
 *
 */

#ifndef PH_STATUS_H

#define PH_STATUS_H 1

/** \defgroup phStatus Status Codes and Return Values
 * \ingroup types
 * \brief Definitions of possible status codes returned by all functions.
 *
 * All functions within the this firmware return a status code which possible values are defined here within
 * this module. In addition to that this module also defines some macros which allows checking against given
 * error codes and perform appropriate action.
 * @{
 */

/*********************************************************************************************************************/
/*   GLOBAL DEFINES                                                                                                  */
/*********************************************************************************************************************/
/** \name Status error check macros
 * @{
 */
/**
 * Macro which checks a given status against #PH_STATUS_SUCCESS. If the status is not equal to #PH_STATUS_SUCCESS then
 * the current function is left with the given status code as return value.
 */
#define PH_STATUS_CHECK_SUCCESS(status)         do {if ((status) != PH_STATUS_SUCCESS) {return (status);}} while (0)
/**
 * Macro which executes a function and checks the return value against #PH_STATUS_SUCCESS. If the calling function does not
 * return #PH_STATUS_SUCCESS then the caller is also left with the callee's status code as return value.
 */
#define PH_STATUS_CHECK_SUCCESS_FCT(status,fct) do {(status) = (fct); PH_STATUS_CHECK_SUCCESS(status);} while (0)
/**
 * @}
 */
/*********************************************************************************************************************/
/*   GLOBAL DATATYPES                                                                                                */
/*********************************************************************************************************************/
/**
 * This is the return type used by mostly all of the functions within this firmware project.
 */
typedef enum
{
  PH_STATUS_SUCCESS                          = 0x00, /**< Value to be returned in case of success. */
  PH_STATUS_UNKNOWN_ERROR                    = 0xFF, /**< Value to be returned in case of Unknown Error. */
  /* GENERIC *******************/
  PH_STATUS_MEMORY_ERROR                     = 0x01,
  PH_STATUS_INTERNAL_ERROR                   = 0x02,
                                                    
  PH_STATUS_TIMEOUT_ERROR                    = 0x03,
  PH_STATUS_CRC_ERROR                        = 0x05,
  PH_STATUS_NOT_FOUND_ERROR                  = 0x07,
  PH_STATUS_BUSY                             = 0x08,
  PH_STATUS_PARAMETER_ERROR                  = 0x09,
  PH_STATUS_UNKNOWN_CMD                      = 0x0B,
  PH_STATUS_ABORTED_CMD                      = 0x0C,
                                                    
  /*****************************/
  PH_STATUS_AT_PRIMARY_ERROR                 = 0x10,
                                                    
  PH_STATUS_AT_SECONDARY_ERROR               = 0x11,
                                                    
  PH_STATUS_AT_FATAL_ERROR                   = 0x12,
  PH_STATUS_KEY_AT1_ERROR                    = 0x13,
  PH_STATUS_KEY_AT2_ERROR                    = 0x14,
  PH_STATUS_NO_DOWNLOAD                      = 0x15,
  PH_STATUS_RFU16                            = 0x16,
  /*****************************/
  PH_STATUS_INTERNAL_ERROR_0                 = 0xC0,
  PH_STATUS_INTERNAL_ERROR_1                 = PH_STATUS_INTERNAL_ERROR_0 + 1,
  PH_STATUS_INTERNAL_ERROR_2                 = PH_STATUS_INTERNAL_ERROR_0 + 2,
  PH_STATUS_INTERNAL_ERROR_3                 = PH_STATUS_INTERNAL_ERROR_0 + 3,
  PH_STATUS_INTERNAL_ERROR_4                 = PH_STATUS_INTERNAL_ERROR_0 + 4,
  PH_STATUS_INTERNAL_ERROR_5                 = PH_STATUS_INTERNAL_ERROR_0 + 5,
  PH_STATUS_INTERNAL_ERROR_6                 = PH_STATUS_INTERNAL_ERROR_0 + 6,
  PH_STATUS_INTERNAL_ERROR_7                 = PH_STATUS_INTERNAL_ERROR_0 + 7,
  PH_STATUS_INTERNAL_ERROR_8                 = PH_STATUS_INTERNAL_ERROR_0 + 8,
  /*****************************/
  /* EECTRL HAL Specific Status Codes */
  PH_STATUS_EECTRL_ECC_BYPASS_SETTING_INCORRECT
  /*****************************/
} phStatus_t;

/**
 * @}
 */

#endif /* PH_STATUS_H */
