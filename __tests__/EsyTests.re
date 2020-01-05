open Jest;

[@bs.val] external __dirname: string = "__dirname";

let projPath = Filename.dirname(__dirname);
describe("Esy.getStatus", () => {
  Expect.(
    testPromise(
      "Checking if running esy status in __dirname works: " ++ projPath, () => {
      Js.Promise.(
        Esy.getStatus(projPath)
        |> then_((status: Esy.status) => {
             expect(status.isProject) |> toBe(false) |> resolve
           })
      )
    })
  )
});
