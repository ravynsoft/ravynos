//
//  IOHIDAnalytics.h
//  IOKitUser
//
//  Created by AB on 11/29/18.
//

#ifndef IOHIDAnalytics_h
#define IOHIDAnalytics_h

#include <CoreFoundation/CoreFoundation.h>

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED
    
typedef struct _HIDAnalyticsHistogramSegmentConfig {
    
    uint8_t  bucket_count;
    uint8_t  bucket_width;  // normalized on scale of value_normalizer
    uint8_t  bucket_base;   // normalized
    uint64_t value_normalizer; // used for normalizing value to be bucketized
    
} IOHIDAnalyticsHistogramSegmentConfig;

/*!
 * IOHIDAnalyticsEventCreate
 *
 * @abstract
 * Creates a HIDAnalyticsEvent object and registers it with HIDAnalyticsReporter.
 *
 * @discussion
 * HIDAnalyticsEvent object is used to log data to analytics server. It is composed of
 * mutable and immutable fields. Immutable fields can be added as part of description
 * during creation. All field added using HIDAnalyticsEventAdd* call are considered to be
 * mutable. Data collection for given event begins after it's activation. User need to
 * cancel event to stop data collection for it.
 *
 * @param eventName
 * Event name to uniquely identify event in CA.
 *
 * @param description
 * Immutable fields associated with event. For example event to log copy property for service
 * API latency can have service description as immutable field.
 *
 * @result
 * Returns an instance of a HIDAnalyticsEvent object on success.
 */
CF_EXPORT
CFTypeRef __nullable IOHIDAnalyticsEventCreate(CFStringRef eventName, CFDictionaryRef _Nullable description);

/*!
 * IOHIDAnalyticsEventAddHistogramField
 *
 * @abstract
 * Add HIDAnalyticsEventField of type Histogram.
 *
 * @discussion
 * Add histogram field to HIDAnalytics event.
 *
 * @param event
 * HIDAnayticsEventRef returned from  HIDAnalyticsEventCreate.
 *
 * @param fieldName
 * Field name to uniquely identify field in event.
 *
 * @param segments
 * Array of segment each containing bucketization information
 *
 * Example
 * Data : 1000, 2000, 2500, 4000, 8000
 *
 * .bucket_count = 5
 * .bucket_width = 2
 * .bucket_base = 1
 * .bucket_normalizer = 1000
 *
 * [ <=1     <=3     <=5   <=7   <=9 ]
 * [  1      2        1     0     1  ]
 *
 * @param count
 * count of segment array.
 */
CF_EXPORT
void IOHIDAnalyticsEventAddHistogramField(CFTypeRef  event, CFStringRef  fieldName, IOHIDAnalyticsHistogramSegmentConfig*  segments, CFIndex count);

/*!
 * IOHIDAnalyticsEventAddField
 *
 * @abstract
 * Add HIDAnalyticsEventField of type Integer.
 * Need to extend this with type option to support
 * normal fields for future
 *
 * @discussion
 * Field is added to event. Value set by user is what it gets.
 * No value processing is done for this field.
 *
 * @param event
 * HIDAnayticsEventRef returned from  HIDAnalyticsEventCreate.
 *
 * @param fieldName
 * Field name to uniquely identify field in event.
 */
CF_EXPORT
void IOHIDAnalyticsEventAddField(CFTypeRef  event, CFStringRef  fieldName);

/*!
 * IOHIDAnalyticsEventActivate
 *
 * @abstract
 * Register event with reporter and start collection of data for it.
 *
 * @discussion
 * Once event is activated, no new field should be added.
 *
 * @param event
 * HIDAnayticsEventRef returned from  HIDAnalyticsEventCreate.
 *
 */
CF_EXPORT
void IOHIDAnalyticsEventActivate(CFTypeRef  event);

/*!
 * IOHIDAnalyticsEventCancel
 *
 * @abstract
 * Stop data collection for given event.
 *
 * @discussion
 * Stop data collection for event and invalidate any resources attached with event.
 *
 * @param event
 * HIDAnayticsEventRef returned from  HIDAnalyticsEventCreate.
 *
 */
CF_EXPORT
void IOHIDAnalyticsEventCancel(CFTypeRef  event);

/*!
 * IOHIDAnalyticsEventSetIntegerValueForField
 *
 * @abstract
 * Set value for given event field
 *
 * @discussion
 * Set value for given event field. Update of field value is based on type of field. For example
 * if field type is histogram then call for given API will bucketize value based on bucket config
 * specified during  HIDAnalyticsEventAdd*Field.
 *
 * @param event
 * HIDAnayticsEventRef returned from  HIDAnalyticsEventCreate.
 *
 * @param fieldName
 * Event field name.
 *
 * @param value
 * Event field value to set.
 *
 */
CF_EXPORT
void IOHIDAnalyticsEventSetIntegerValueForField(CFTypeRef  event, CFStringRef  fieldName, uint64_t value);
    

/*!
 * IOHIDAnalyticsHistogramEventCreate
 *
 * @abstract
 * Creates a HIDAnalyticsHistogramEventCreate object and registers it with HIDAnalyticsReporter.
 *
 * @discussion
 * HIDAnalyticsHistogramEventCreate object is used to log data to analytics server. It has
 * single histogram field. HIDAnalyticsEventAdd* is not supported. Main difference between IOHIDAnalyticsEventCreate +
 * IOHIDAnalyticsEventAddHistogramField and HIDAnalyticsHistogramEventCreate is in terms of performance.
 * If Event has single histogram field then it;s good to use IOHIDAnalyticsHistogramEventCreate. Another difference is
 * in term of number of histogram fields you can add, for normal event you can add multiple number of histogram fields wheras
 * HIDAnalyticsHistogramEvent supports single histogram field. It is recommended to use this API if you are on performance critical path.
 *
 * @param eventName
 * Event name to uniquely identify event in CA.
 *
 * @param description
 * Immutable fields associated with event. For example event to log copy property for service
 * API latency can have service description as immutable field.
 *
 * @param fieldName
 * Field name to uniquely identify field in event.
 *
 * @param segments
 * Array of segment each containing bucketization information
 *
 * Example
 * Data : 1000, 2000, 2500, 4000, 8000
 *
 * .bucket_count = 5
 * .bucket_width = 2
 * .bucket_base = 1
 * .bucket_normalizer = 1000
 *
 * [ <=1     <=3     <=5   <=7   <=9 ]
 * [  1      2        1     0     1  ]
 *
 * @param count
 * count of segment array.
 *
 * @result
 * Returns an instance of a HIDAnalyticsHistogramEventobject on success.
 */
CF_EXPORT
CFTypeRef __nullable IOHIDAnalyticsHistogramEventCreate(CFStringRef eventName, CFDictionaryRef _Nullable description, CFStringRef fieldName,IOHIDAnalyticsHistogramSegmentConfig*  segments, CFIndex count);

/*!
 * IOHIDAnalyticsHistogramEventSetIntegerValue
 *
 * @abstract
 * Set value  for HIDAnalyticsHistogramEvent
 *
 * @discussion
 * Set value for given histogram event . Not supported for  event with multiple fields.
 *
 * @param event
 * HIDAnalyticsHistogramEventRef returned from  HIDAnalyticsHistogramEventCreate.
 *
 * @param value
 * Event field value to set.
 *
 */
CF_EXPORT
void IOHIDAnalyticsHistogramEventSetIntegerValue(CFTypeRef  event, uint64_t value);
    
CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* IOHIDAnalytics_h */

