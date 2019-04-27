while (1) {
   print("> ");
   var buffer = getLine();
   if ("exit" == buffer) {
      exit();
   }
   try {
      printLine(eval(buffer));
   } catch (e) {
      printLine(e);
   }
}