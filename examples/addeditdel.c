#include <stdio.h>
#include <gcalendar.h>

int main(int argc, char *argv[])
{
        gcal_t gcal;
        gcal_event_t event;
        int result;

        /* Create a gcal 'object' and authenticate with server */
        if (!(gcal = gcal_new(GCALENDAR)))
                exit(1);

        if (argc == 3)
                result = gcal_get_authentication(gcal, argv[1], argv[2]);
        else
                result = gcal_get_authentication(gcal, "username",
                                                 "password");

        /* Create an event 'object' and fill in some data */
        if ((event = gcal_event_new(NULL))) {
                gcal_event_set_title(event, "A new event");
                gcal_event_set_content(event, "Here goes the description");
                gcal_event_set_start(event, "2008-06-26T16:00:00Z");
                gcal_event_set_end(event, "2008-06-26T18:00:00Z");
                gcal_event_set_where(event, "A nice place for a meeting");
        }

        /* Add a new event */
        if (!(result = gcal_add_event(gcal, event))) {

                /* Edit this event */
                gcal_event_set_title(event, "Changing the title");
                result = gcal_update_event(gcal, event);

                /* Delete this event (note: google doesn't really deletes
                 * the event, but set its status to 'cancelled' and keeps
                 * then for nearly 4 weeks).
                 */
                result = gcal_erase_event(gcal, event);
        }

        /* Cleanup */
        gcal_event_delete(event);
        gcal_delete(gcal);
        gcal_final_cleanup();

        return 0;
}

