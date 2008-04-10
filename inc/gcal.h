/**
 * @file   gcal.h
 * @author Adenilson Cavalcanti da Silva <adenilson.silva@indt.org.br>
 * @date   Mon Mar  3 20:15:09 2008
 *
 * @brief  libgcal header file.
 *
 * This defines the public functions.
 *
 */
#ifndef __GCAL_LIB__
#define __GCAL_LIB__

/* For size_t */
#include <stdlib.h>

/** Library structure. It holds resources (curl, buffer, etc).
 */
struct gcal_resource;

/** Library structure, represents each calendar event entry.
 */
struct gcal_entries {
	/** The 'what' field */
	char *title;
	/** element ID */
	char *id;
	/** The edit URL */
	char *edit_uri;
	/** The event description */
	char *content;
	/** If the event is recurrent */
	char *dt_recurrent;
	/** When/start time */
	char *dt_start;
	/** When/end time */
	char *dt_end;
	/** Location of event */
	char *where;
	/** Event status */
	char *status;
	/** Time when the event was updated. */
	char *updated;
};

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


/** Dumps events from default calendar to internal buffer.
 * \todo Let the library user select which calendar he/she wants
 * to get the events. See \ref gcal_calendar_list.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int gcal_dump(struct gcal_resource *ptr_gcal);

/** Get a list of users calendars (gcalendar supports multiple calendars
 * besides the default calendar).
 *
 * I think it would be a good idea to let the library user decide which
 * calendar to get the events. See too \ref gcal_dump.
 *
 * \todo Parse the Atom feed and provide easy access to the calendar lists.
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return Returns 0 on success, -1 otherwise.
 */
int gcal_calendar_list(struct gcal_resource *ptr_gcal);


/** Return the number of event entries a calendar has (you should
 * had got the atom stream before, using \ref gcal_dump).
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return -1 on error, any number >= 0 otherwise.
 */
int gcal_entries_number(struct gcal_resource *ptr_gcal);


/** Extracts from the atom stream the calendar event entries (you should
 * had got the atom stream before, using \ref gcal_dump).
 *
 * Pay attention that it returns a vector of structures that must be destroyed
 * using \ref gcal_destroy_entries.
 *
 * Since atom XML feeds can get huge, as soon the function creates entries
 * vector and copies the data from the internal \ref gcal_resource buffer,
 * it will free its internal buffer to save memory.
 *
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @param length Pointer to an int, it will have the vector length.
 *
 * @return A pointer on sucess, NULL otherwise.
 */
struct gcal_entries *gcal_get_entries(struct gcal_resource *ptr_gcal,
				      size_t *length);

/** Cleanup memory of 1 entry structure pointer.
 *
 *
 * @param entry A pointer to a \ref gcal_entries.
 */
void gcal_destroy_entry(struct gcal_entries *entry);

/** Cleanup the memory of a vector of calendar entries created using
 * \ref gcal_get_entries.
 *
 * @param entries A pointer to a vector of \ref gcal_entries structure.
 *
 * @param length The vector length.
 */
void gcal_destroy_entries(struct gcal_entries *entries, size_t length);


/** Creates an new calendar event.
 *
 * You need to first succeed to get an authorization token using
 * \ref gcal_get_authentication.
 *
 * @param entries A pointer to an calendar entry event (see \ref gcal_entries).
 *
 * @param ptr_gcal Pointer to a \ref gcal_resource structure, which has
 *                 previously got the authentication using
 *                 \ref gcal_get_authentication.
 *
 * @return -1 on error, 0 on success.
 */
int gcal_create_event(struct gcal_entries *entries,
		      struct gcal_resource *ptr_gcal);

#endif
