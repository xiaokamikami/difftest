package difftest
import java.io._
import scala.collection.mutable
import chisel3.experimental.treedump
import chisel3.Bool
import ujson.True
import upickle.default

object DataCollector {
  private val data: mutable.Map[String, mutable.Map[String, Any]] = mutable.Map()
  var diffEnableJson = false

  def diffJsonEnabled() = {
    diffEnableJson = true
  }

//   def update(className: String, params: Map[String, Any], count: Boolean): Unit = {
//     val currentData = data.getOrElseUpdate(className, mutable.Map("exception_count" -> "0"))
//     val currentCount = currentData("exception_count").toInt
//     if (count == true)
//         currentData("exception_count") = (currentCount + 1).toString

//     params.foreach { case (k, v) =>
//       currentData(k) = v.toString
//     }
//   }
  def update(className: String, params: Map[String, Any], count: Boolean): Unit = {
    val currentData = data.getOrElseUpdate(className, mutable.Map("exception_count" -> 0))
    if (count) {
      val currentCount = currentData("exception_count").asInstanceOf[Int]
      currentData("exception_count") = currentCount + 1
    }

    params.foreach { case (k, v) =>
      currentData(k) = v
    }
  }

  def writeToFile(filePath: String): Unit = {
    // val jsonString = data.map { case (className, params) =>
    //   val paramsString = params.map { case (k, v) => s""""$k": "$v"""" }.mkString(", ")
    //   s""""$className": { $paramsString }"""
    // }.mkString("{ ", ", ", " }")
    // val jsonString = data.map { case (className, params) =>
    //   val paramsString = params.map { case (k, v) => s"""    "$k": "$v"""" }.mkString(",\n")
    //   s"""  "$className": {\n$paramsString\n  }"""
    // }.mkString("{\n", ",\n", "\n}")
    val jsonString = data.map { case (className, params) =>
      val paramsString = params.map { case (k, v) =>
        val valueStr = v match {
          case s: String => s""""$s""""
          case _ => v.toString
        }
        s""""$k": $valueStr"""
      }.mkString(",\n    ")
      s""""$className": {\n    $paramsString\n  }"""
    }.mkString("{\n", ",\n", "\n}")

    val file = new File(filePath)
    val writer = new PrintWriter(new FileWriter(file))
    try {
      writer.println(jsonString)
    } finally {
      writer.close()
    }
  }

// Register the JVM shutdown hook
  sys.addShutdownHook {
    println("Shutdown hook triggered, writing data to file...")
    writeToFile("difftest/diff_class_info.json")
  }
}
