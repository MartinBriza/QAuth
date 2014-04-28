Qt user authentication library

TODO:

* PAM backend seems to be a bit slow when opening a session

* test and release

### Current status

Working, it's possible to use the lib for an actual application.

API stabilized, release coming soon

PAM prompts are predicted and sent in batches

### Examples

Only proofs of concept, not intended for any real usage

minimaldm - DM that just logs a user into some desktop session... both values are hard-coded into the source

checkpass - PAM conversation in the terminal

qmlapp - PAM conversation in an ugly QML application with horrible user experience (you have to press Return AND click on all input boxes)