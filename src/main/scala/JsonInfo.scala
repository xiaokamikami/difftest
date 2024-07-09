package difftest.json

import java.io._
import scala.collection.mutable
import ujson.True
import upickle.default
import scala.io.Source

object DataCollector {
  private val data: mutable.Map[String, mutable.Map[String, Any]] = mutable.Map()

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
  def readFromFile(filePath: String): Unit = {
    val source = Source.fromFile(filePath)
    val jsonString =
      try source.mkString
      finally source.close()

    // A simple JSON parser
    val jsonPattern = """\s*"(\w+)"\s*:\s*(\d+|".*?")\s*""".r
    val classPattern = """\s*"(\w+)"\s*:\s*\{\s*(.*?)\s*\}\s*""".r

    classPattern.findAllMatchIn(jsonString).foreach { classMatch =>
      val className = classMatch.group(1)
      val paramsString = classMatch.group(2)

      val params = mutable.Map[String, Any]()
      jsonPattern.findAllMatchIn(paramsString).foreach { paramMatch =>
        val key = paramMatch.group(1)
        val value = paramMatch.group(2)
        val parsedValue = if (value.startsWith("\"")) value.stripPrefix("\"").stripSuffix("\"") else value.toInt
        params(key) = parsedValue
      }

      data(className) = params
    }
  }

// Find json key
  def query(className: String, key: String): Option[Any] = {
    data.get(className).flatMap(_.get(key))
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
