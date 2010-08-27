#include <stdio.h>
#include <gcalendar.h>

int main(int argc, char *argv[])
{
        gcal_t gcal;
        struct gcal_event_array event_array;
        int result;

        /* Create a gcal 'object' and authenticate with server */
        if (!(gcal = gcal_new(GCALENDAR)))
                exit(1);

        if (argc == 3)
                result = gcal_get_authentication(gcal, argv[1], argv[2]);
        else
                result = gcal_get_authentication(gcal, "username",
                                                 "password");

        /* This will query for all updated events (fall in this category
         * added/deleted/updated events) starting for 06:00Z UTC of today).
         */
        if (!(result = gcal_get_updated_events(gcal, &event_array, NULL)))
                printf("updated events: %d\n", (int)event_array.length);

        /* Cleanup */
        gcal_cleanup_events(&event_array);
        gcal_delete(gcal);

        return 0;

}

