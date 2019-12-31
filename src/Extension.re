open Vscode;
open LSP;

let createClient = (~id, ~name, ~folder) =>
  Js.Promise.(
    Server.make(folder.Folder.uri.fsPath)
    |> then_(serverOptions => {
         Ok(
           LanguageClient.make(
             ~id,
             ~name,
             ~serverOptions,
             ~clientOptions=Client.make(),
           ),
         )
         |> resolve
       })
  );

let activate = context => {
  MultiWorkspace.start(
    ~context, ~commands=[||], ~createClient=(_document, folder) => {
    Js.Promise.(
      createClient(
        ~id="merlin-language-server",
        ~name="Merlin Language Server",
        ~folder,
      )
      |> catch(e => {
           let message = Bindings.Error.ofPromiseError(e);
           Js.Promise.resolve(Error(message));
         })
    )
  });
};
