
import scala.io.Source

/** Reads a file and returns an array of the lines.
 */
def readFile(filename: String): Array[String] = {
    return Source.fromFile(filename).getLines.toArray
}

/** Converts an option to an int with a safe default.
 */
def optionToInt(x: Option[Integer]): Integer = {
    return x match {
        case Some(i) => i
        case _ => -1
    }
}

@main def exec(inDirectory: String, inFile: String) = {
   importCode(inputPath=inDirectory, projectName="project")
   val lines = readFile("/shared/data/loop_exchange.c")
   val loop = cpg.method.name("main").controlStructure.controlStructureType("FOR").l(0)

   val lastStatement = cpg.method.name("main").controlStructure.controlStructureType("FOR").ast.sortBy(_.lineNumber.getOrElse("-1").toString.toInt).l.last
   val endOfLastStatementInBlockContent = optionToInt(lastStatement.columnNumber)
   var beginIdx = loop.code.indexOf('(')+1
   var endIdx = beginIdx + loop.code.slice(beginIdx, loop.code.length()).indexOf(';') + 1
   val initStmt = loop.code.slice(beginIdx, endIdx).trim
   beginIdx = endIdx
   endIdx = beginIdx + loop.code.slice(beginIdx, loop.code.length()).indexOf(';') + 1
   val condExpr = loop.code.slice(beginIdx, endIdx).trim.stripSuffix(";")
   beginIdx = endIdx
   endIdx = beginIdx + loop.code.slice(beginIdx, loop.code.length()).indexOf(')')
   val postStmt = loop.code.slice(beginIdx, endIdx).trim + ";"

   val loopLineNumber = optionToInt(loop.lineNumber)
   val lastStatementLineNumber = optionToInt(lastStatement.lineNumber)
   val newCodeBuilder = new StringBuilder()
   for((line,i) <- lines.view.zipWithIndex) {
       if (i == loopLineNumber-1) {
           newCodeBuilder ++= " " * 4 + initStmt + "\n" // Add the init expr before the loop
           val beginOfForStmt = optionToInt(loop.columnNumber)
           val endOfForStmt = beginOfForStmt + loop.code.length()
           newCodeBuilder ++= line.slice(0, beginOfForStmt) + "while (" + condExpr + ")" + line.slice(endOfForStmt + 1, line.length()) + "\n"
       }
       else if (i == lastStatementLineNumber-1) {
           newCodeBuilder ++= line + "\n" // Add the last stmt as-is
           newCodeBuilder ++= " " * 8 + postStmt + "\n" // Append the post expr as a new stmt
       }
       else {
           newCodeBuilder ++= line + "\n"
       }
   }
   val newCode = newCodeBuilder.toString()
   print(newCode)
}
