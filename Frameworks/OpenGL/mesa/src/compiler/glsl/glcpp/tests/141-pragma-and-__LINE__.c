Line 1 /* Test for a bug where #pragma was throwing off the __LINE__ count. */
Line __LINE__ /* Line 2 */
#pragma Line 3
Line __LINE__ /* Line 4 */
#pragma Line 5
Line __LINE__ /* Line 6 */
