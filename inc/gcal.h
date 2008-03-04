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


#endif
