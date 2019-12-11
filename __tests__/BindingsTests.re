open Jest;

describe("Expect", () => {
  Expect.(
    testPromise("toBe", () => {
      Js.Promise.(
        Bindings.ChildProcess.exec(
          "echo hey",
          Bindings.ChildProcess.Options.make(),
        )
        |> then_(((stdout, _)) => {
             expect(stdout) |> toBe("hey\n") |> resolve
           })
      )
    })
  )
});
