package systemcmds

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"pnconnector/src/routers/m9k/model"
	"pnconnector/src/util"
	"time"

	"cli/cmd/displaymgr"
	"cli/cmd/messages"

	"github.com/labstack/gommon/log"
	"github.com/spf13/cobra"
)

var StartSystemCmd = &cobra.Command{
	Use:   "start",
	Short: "Start PoseidonOS.",
	Long: `Start PoseidonOS.

Syntax:
	poseidonos-cli system start .
          `,
	Run: func(cmd *cobra.Command, args []string) {

		var command = "RUNIBOFOS"

		startSystemReq := messages.Request{
			RID:     "fromfakeclient",
			COMMAND: command,
		}

		reqJSON, err := json.Marshal(startSystemReq)
		if err != nil {
			log.Debug("error:", err)
		}

		displaymgr.PrintRequest(string(reqJSON))

		// TODO(mj): Here, we execute a script to run POS. This needs to be revised in the future.
		res := model.Response{}

		fmt.Println("Start system...")

		// TODO(mj): Although go test for this command will be passed,
		// it will print out some error commands because of the file path
		// to the execution script. This needs to be fixed later.
		path, _ := filepath.Abs(filepath.Dir(os.Args[0]))
		startCmd := fmt.Sprintf("/../script/start_poseidonos.sh")
		err = util.ExecCmd(path+startCmd, false)

		fmt.Println(err)

		if err != nil {
			res.Result.Status.Code = 11000
			fmt.Println("PoseidonOS has failed to start with error code: ", res.Result.Status.Code)
		} else {
			res.Result.Status.Code = 0
			res.LastSuccessTime = time.Now().UTC().Unix()
			fmt.Println("PoseidonOS has successfully started")
		}

	},
}

func init() {

}

func PrintResponse(response string) {
	fmt.Println(response)
}
