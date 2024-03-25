# mawimctl
MaWiM Control - Protocol for controlling MaWiM. For comprehensive documentation
see `../doc/mawimctl.md`

## Usage
**Synposis:** `mawimctl <command [command options]>`

### Commands
* `close_focused` 
* `get_version`
* `get_workspace`
* `move_focused_to_workspace <workspace number>`
* `reload`
* `set_workspace <workspace number>`

### Environment Variables
* `MAWIMCTL_SOCK` Specifies the location for the mawimctl socket in the filesystem.

## License
mawimctl is part of MaWiM and thus also falls under the same BSD 3-Clause license found in the repositories root-directory.