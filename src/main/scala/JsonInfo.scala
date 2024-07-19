package difftest.json

import java.io._
import scala.collection.mutable
import ujson._
import upickle.default
import scala.io.Source

object DataCollector {
  private val data: mutable.Map[String, mutable.Map[String, Any]] = mutable.Map()

  def update(className: String, params: Map[String, Any], count: Boolean): Unit = {
    if (dumpEnableJson) {
      val currentData = data.getOrElseUpdate(className, mutable.Map("exception_count" -> 0))
      if (count) {
        val currentCount = currentData("exception_count").asInstanceOf[Int]
        currentData("exception_count") = currentCount + 1
      }
      params.foreach { case (k, v) =>
        currentData(k) = v
      }
    }
  }

  def writeToFile(filePath: String): Unit = {
    val jsonString = data.map { case (className, params) =>
      val paramsString = params.map { case (k, v) =>
        val valueStr = v match {
          case s: String => s""""$s""""
          case _         => v.toString
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

// Load json file
  def readFromFile(filePath: String): mutable.Map[String, mutable.Map[String, Any]] = {
    val source = Source.fromFile(filePath)
    val jsonString = try source.mkString finally source.close()
    val parsedData = ujson.read(jsonString).obj

    mutable.Map(parsedData.map { case (className, classParams) =>
      val paramsMap = mutable.Map(classParams.obj.map { case (k, v) => k -> v.value }.toSeq: _*)
      className -> paramsMap
    }.toSeq: _*)
  }

  var dumpEnableJson = false

  def diffJsonEnabled() = {
    dumpEnableJson = true
  }

// Register the JVM shutdown hook
  sys.addShutdownHook {
    if (dumpEnableJson) {
      println("Shutdown hook triggered, writing data to file...")
      writeToFile("difftest/diff_class_info.json")
    }
  }
}
