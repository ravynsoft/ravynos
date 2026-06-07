/*
******************************************************************************
* Copyright (C) 2006-2008, 2017-2018 Apple Inc. All Rights Reserved.
******************************************************************************
*/

#ifndef URBTOK_H
#define URBTOK_H

#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include "unicode/ubrk.h"
#include "unicode/parseerr.h"

/**
 * The interfaces here are meant to extend the functionality of the standard
 * ubrk_* interfaces in ubrk.h to allow for faster batch tokenization. This
 * was primarily intended for Spotlight and related processes. There are two
 * versions of these:
 *
 * The versions prefixed urbtok_ extend the standard ICU RuleBasedBreakIterator
 * class. These are intended to fully support all of the current rule syntax used
 * by that class, and should urbtok_tokenize give results equivalent to a loop using a
 * combination of the standard functions ubrk_next to get the next break (determining
 * the length of the previous token) and ubrk_getRuleStatusVec to get a flag value
 * formed as the bitwise OR of all of the values in the returnend vector, skipping all
 * tokens whose flag value is -1. urbtok_tokenize is faster than such a loop since it
 * assumes only one pass over the text in the forward direction, and shut skips caching
 * of breaks positions and makes other simplifying assumptions. However, it may not be
 * fast enough fo Spotlight.
 *
 * Thus we also include the versions prefixed by urbtok57_, which use a legacy ICU 57
 * version of RuleBasedBreakIterator and an Apple subclass RuleBasedTokenizer. These
 * versions do not support any RuleBasedBreakIterator rule sytax enhancements from
 * later than ICU 57.
 *
 * The two different sets of functions should not be mixed; urbtok57_getBinaryRules
 * should only be used with a UBreakIterator created using urbtok57_openRules;
 * urbtok57_tokenize should only be used with a UBreakIterator created using
 * urbtok57_openRules or urbtok_openBinaryRules[NoCopy], etc. Similarly, the
 * urbtok_ functions should only be used with other urbtok_ functions.
 */
 
/**
 * struct for returning token results
 */
typedef struct RuleBasedTokenRange {
    signed long location;
    signed long length;
} RuleBasedTokenRange;

/**
 * Open a new UBreakIterator for locating text boundaries for a specified locale.
 * A UBreakIterator may be used for detecting character, line, word,
 * and sentence breaks in text.
 * @param type The type of UBreakIterator to open: one of UBRK_CHARACTER, UBRK_WORD,
 * UBRK_LINE, UBRK_SENTENCE
 * @param locale The locale specifying the text-breaking conventions. Note that
 * locale keys such as "lb" and "ss" may be used to modify text break behavior,
 * see general discussion of BreakIterator C API.
 * @param status A UErrorCode to receive any errors.
 * @return A UBreakIterator for the specified type and locale.
 * @see ubrk_open
 * @internal
 */
U_INTERNAL UBreakIterator* U_EXPORT2
urbtok_open(UBreakIteratorType type,
           const char *locale,
           UErrorCode *status);

/**
 * Open a new UBreakIterator for tokenizing text using specified breaking rules.
 * The rule syntax is ... (TBD)
 * @param rules A set of rules specifying the text breaking conventions.
 * @param rulesLength The number of characters in rules, or -1 if null-terminated.
 * @param parseErr   Receives position and context information for any syntax errors
 *                   detected while parsing the rules.
 * @param status A UErrorCode to receive any errors.
 * @return A UBreakIterator for the specified rules.
 * @see ubrk_open
 * @internal
 */
U_INTERNAL UBreakIterator* U_EXPORT2
urbtok_openRules(const UChar     *rules,
               int32_t         rulesLength,
               UParseError     *parseErr,
               UErrorCode      *status);

/**
 * Open a new UBreakIterator for tokenizing text using specified breaking rules.
 * @param rules A set of rules specifying the text breaking conventions. The binary rules
 *              must be at least 32-bit aligned. Note: This version makes a copy of the
 *				rules, so after calling this function the caller can close or release
 *				the rules that were passed to this function. The copy created by this
 *				call will be freed when ubrk_close() is called on the UBreakIterator*.
 * @param status A UErrorCode to receive any errors.
 * @return A UBreakIterator for the specified rules.
 * @see ubrk_open
 * @internal
 */
U_INTERNAL UBreakIterator* U_EXPORT2
urbtok_openBinaryRules(const uint8_t *rules,
               UErrorCode      *status);

/**
 * Open a new UBreakIterator for tokenizing text using specified breaking rules.
 * @param rules A set of rules specifying the text breaking conventions. The binary rules
 *              must be at least 32-bit aligned. Note: This version does NOT make a copy
 *				of the rules, so after calling this function the caller must not close or
 *				release the rules passed to this function until after they are finished
 *				with this UBreakIterator* (and any others created using the same rules)
  *				and have called ubrk_close() to close the UBreakIterator* (and any others
 *				using the same rules).
 * @param status A UErrorCode to receive any errors.
 * @return A UBreakIterator for the specified rules.
 * @see ubrk_open
 * @internal
 */
U_INTERNAL UBreakIterator* U_EXPORT2
urbtok_openBinaryRulesNoCopy(const uint8_t *rules,
               UErrorCode      *status);

/**
 * Get the (native-endian) binary break rules for this tokenizer.
 * @param bi The tokenizer to use.
 * @param buffer The output buffer for the rules. You can pass 0 to get the required size.
 * @param buffSize The size of the output buffer.
 * @param status A UErrorCode to receive any errors.
 * @return The actual size of the binary rules, whether they fit the buffer or not.
 * @internal
 */
U_INTERNAL uint32_t U_EXPORT2
urbtok_getBinaryRules(UBreakIterator      *bi,
                uint8_t             *buffer,
                uint32_t            buffSize,
                UErrorCode          *status);

/**
 * Tokenize text using a rule-based tokenizer.
 * This is primarily intended for speedy batch tokenization using very simple rules.
 * It does not currently implement support for all of the features of ICU break rules
 * (adding that would reduce performance). If you need support for all of the ICU rule
 * features, please use the standard ubrk_* interfaces; instead of urbtok_tokenize,
 * use a loop with ubrk_next and ubrk_getRuleStatus.
 *
 * @param bi The tokenizer to use.
 * @param maxTokens The maximum number of tokens to return.
 * @param outTokens An array of RuleBasedTokenRange to fill in with the tokens.
 * @param outTokenFlags An (optional) array of uint32_t to fill in with token flags.
 * @return The number of tokens returned, 0 if done.
 * @internal
 */
U_INTERNAL int32_t U_EXPORT2
urbtok_tokenize(UBreakIterator      *bi,
               int32_t              maxTokens,
               RuleBasedTokenRange  *outTokens,
               unsigned long        *outTokenFlags);

/**
 * Swap the endianness of a set of binary break rules.
 * @param rules A set of rules which need swapping.
 * @param buffer The output buffer for the swapped rules, which must be the same
 *               size as the input rules buffer.
 * @param inIsBigEndian UBool indicating whether the input is big-endian
 * @param outIsBigEndian UBool indicating whether the output should be big-endian
 * @param status A UErrorCode to receive any errors.
 * @internal
 */
U_INTERNAL void U_EXPORT2
urbtok_swapBinaryRules(const uint8_t *rules,
               uint8_t          *buffer,
               UBool            inIsBigEndian,
               UBool            outIsBigEndian,
               UErrorCode       *status);



/**
 * Open a new UBreakIterator for tokenizing text using specified breaking rules.
 * The rule syntax is ... (TBD)
 * @param rules A set of rules specifying the text breaking conventions.
 * @param rulesLength The number of characters in rules, or -1 if null-terminated.
 * @param parseErr   Receives position and context information for any syntax errors
 *                   detected while parsing the rules.
 * @param status A UErrorCode to receive any errors.
 * @return A UBreakIterator for the specified rules.
 * @see ubrk_open
 * @internal
 */
U_INTERNAL UBreakIterator* U_EXPORT2
urbtok57_openRules(const UChar     *rules,
               int32_t         rulesLength,
               UParseError     *parseErr,
               UErrorCode      *status);

/**
 * Open a new UBreakIterator for tokenizing text using specified breaking rules.
 * @param rules A set of rules specifying the text breaking conventions. The binary rules
 *              must be at least 32-bit aligned. Note: This version makes a copy of the
 *				rules, so after calling this function the caller can close or release
 *				the rules that were passed to this function. The copy created by this
 *				call will be freed when ubrk_close() is called on the UBreakIterator*.
 * @param status A UErrorCode to receive any errors.
 * @return A UBreakIterator for the specified rules.
 * @see ubrk_open
 * @internal
 */
U_INTERNAL UBreakIterator* U_EXPORT2
urbtok57_openBinaryRules(const uint8_t *rules,
               UErrorCode      *status);

/**
 * Open a new UBreakIterator for tokenizing text using specified breaking rules.
 * @param rules A set of rules specifying the text breaking conventions. The binary rules
 *              must be at least 32-bit aligned. Note: This version does NOT make a copy
 *				of the rules, so after calling this function the caller must not close or
 *				release the rules passed to this function until after they are finished
 *				with this UBreakIterator* (and any others created using the same rules)
  *				and have called ubrk_close() to close the UBreakIterator* (and any others
 *				using the same rules).
 * @param status A UErrorCode to receive any errors.
 * @return A UBreakIterator for the specified rules.
 * @see ubrk_open
 * @internal
 */
U_INTERNAL UBreakIterator* U_EXPORT2
urbtok57_openBinaryRulesNoCopy(const uint8_t *rules,
               UErrorCode      *status);

/**
 * Get the (native-endian) binary break rules for this tokenizer.
 * @param bi The tokenizer to use.
 * @param buffer The output buffer for the rules. You can pass 0 to get the required size.
 * @param buffSize The size of the output buffer.
 * @param status A UErrorCode to receive any errors.
 * @return The actual size of the binary rules, whether they fit the buffer or not.
 * @internal
 */
U_INTERNAL uint32_t U_EXPORT2
urbtok57_getBinaryRules(UBreakIterator      *bi,
                uint8_t             *buffer,
                uint32_t            buffSize,
                UErrorCode          *status);

/**
 * Tokenize text using a rule-based tokenizer.
 * This is primarily intended for speedy batch tokenization using very simple rules.
 * It does not currently implement support for all of the features of ICU break rules
 * (adding that would reduce performance). If you need support for all of the ICU rule
 * features, please use the standard Apple urbtok_tokenize, or a loop with standard
 * ICU interfaes ubrk_next and ubrk_getRuleStatusVec.
 *
 * @param bi The tokenizer to use.
 * @param maxTokens The maximum number of tokens to return.
 * @param outTokens An array of RuleBasedTokenRange to fill in with the tokens.
 * @param outTokenFlags An (optional) array of uint32_t to fill in with token flags.
 * @return The number of tokens returned, 0 if done.
 * @internal
 */
U_INTERNAL int32_t U_EXPORT2
urbtok57_tokenize(UBreakIterator      *bi,
               int32_t              maxTokens,
               RuleBasedTokenRange  *outTokens,
               unsigned long        *outTokenFlags);

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */

#endif
