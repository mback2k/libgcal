#include <stdio.h>
#include <gcalendar.h>
#include <gcontact.h>

int main(int argc, char *argv[])
{
        gcal_t gcal;
        gcal_contact_t contact;
        struct gcal_contact_array all_contacts;
        int result;
        size_t i;

        /* Create a gcal 'object' and authenticate with server */
        if (!(gcal = gcal_new(GCONTACT)))
                exit(1);
        if (argc == 3)
                result = gcal_get_authentication(gcal, argv[1], argv[2]);
        else
                result = gcal_get_authentication(gcal, "username",
                                                 "password");

	if (result) {
		printf("Failed authentication, exiting...\n");
		return -1;
	}

        /* Get all contacts and print its name/prefered email/updated time */
        result = gcal_get_contacts(gcal, &all_contacts);

        for (i = 0; i < all_contacts.length; ++i) {
                contact = gcal_contact_element(&all_contacts, i);
                if (!contact)
                        break;

                printf("contact: %d\ttitle:%s\temail:%s\tupdated:%s\n",
                       i,
                       gcal_contact_get_title(contact),
                       gcal_contact_get_email(contact),
                       gcal_contact_get_updated(contact));

        }

        /* Cleanup */
        gcal_cleanup_contacts(&all_contacts);
        gcal_delete(gcal);
        gcal_final_cleanup();

        return 0;
}

