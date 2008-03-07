/**
 * @file   gcal.h
 * @author teste
 * @date   Mon Mar  3 20:15:09 2008
 *
 * @brief  libgcal header file.
 *
 * This defines the public functions.
 *
 */
#ifndef __GCAL_LIB__
#define __GCAL_LIB__

/** Library structure. It holds resources (curl, buffer, etc).
 */
struct gcal_resource;

/** Library structure destructor (use it free its internal resources properly).
 */
void gcal_destroy(struct gcal_resource *gcal_obj);

/** Library structure constructor, the user can only have pointers to the
 * library \ref gcal_resource structure.
 */
struct gcal_resource *gcal_initialize(void);

/** Gets from google an authentication token, using the 'ClientLogin' service.
 *
 * Use it before getting/setting google calendar events.
 *
 * @param user The user google login account.
 * @param password User password in plain text
 *                 \todo think in a way to encrypt password
 * @param ptr_gcal Pointer to a library resource structure \ref gcal_resource
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int gcal_get_authentication(char *user, char *password,
			    struct gcal_resource *ptr_gcal);


/** Parses the returned HTML page and extracts the redirection URL
 * that has the Atom feed.
 *
 * \todo Save the xmlDoc structure for future reuse (maybe within
 * structure \ref gcal_resource).
 *
 * @param data Raw data (the HTML page).
 * @param length Data buffer length.
 * @param url Pointer to the pointer (ouch!) that will receive the URL
 * (you should cleanup its memory). It will point to NULL if there is
 * not a URL in the raw data buffer.
 *
 */
void get_the_url(char *data, int length, char **url);


/** Dumps all events to internal buffer.
 *
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int gcal_dump(struct gcal_resource *ptr_gcal);


#endif
