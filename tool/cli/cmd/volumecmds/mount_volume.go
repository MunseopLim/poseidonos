package volumecmds

import (
	"encoding/json"
	"pnconnector/src/log"

	"cli/cmd/displaymgr"
	"cli/cmd/globals"
	"cli/cmd/messages"
	"cli/cmd/socketmgr"

	"github.com/spf13/cobra"
)

var MountVolumeCmd = &cobra.Command{
	Use:   "mount [flags]",
	Short: "Mount a volume to Host.",
	Long: `Mount a volume to Host.

Syntax:
	mount --volume-name VolumeName (--array-name | -a) VolumeName [--subnqn TargetNVMSubsystemNVMeQualifiedName]

Example: 
	poseidonos-cli volume mount --volume-name Volume0 --array-name Volume0
	
         `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "MOUNTVOLUME"

		mountVolumeParam := messages.MountVolumeParam{
			VOLUMENAME: mount_volume_volumeName,
			SUBNQN:     mount_volume_subNqnName,
			ARRAYNAME:  mount_volume_arrayName,
		}

		mountVolumeReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
			PARAM:   mountVolumeParam,
		}

		reqJSON, err := json.Marshal(mountVolumeReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		socketmgr.Connect()
		resJSON := socketmgr.SendReqAndReceiveRes(string(reqJSON))
		socketmgr.Close()

		displaymgr.PrintResponse(command, resJSON, globals.IsDebug, globals.IsJSONRes)
	},
}

// Note (mj): In Go-lang, variables are shared among files in a package.
// To remove conflicts between variables in different files of the same package,
// we use the following naming rule: filename_variablename. We can replace this if there is a better way.
var mount_volume_volumeName = ""
var mount_volume_arrayName = ""
var mount_volume_subNqnName = ""

func init() {
	MountVolumeCmd.Flags().StringVarP(&mount_volume_volumeName, "volume-name", "", "", "The name of the volume to mount")
	MountVolumeCmd.Flags().StringVarP(&mount_volume_arrayName, "array-name", "a", "", "The name of the array where the volume belongs to")
	MountVolumeCmd.Flags().StringVarP(&mount_volume_subNqnName, "subnqn", "", "", "NVMe qualified name of target NVM subsystem")
}
