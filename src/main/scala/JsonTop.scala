package difftest.json

import chisel3._
import chisel3.util._
import difftest._
import difftest.gateway._
import difftest.json._
import ujson._
import scala.collection.mutable
import scala.collection.mutable.ListBuffer
import chisel3.reflect.DataMirror

class JsonTop(config: GatewayConfig) extends Module {
  // Instantiate the endpoint
  val endpoint = DiffJsonTop.creat_top(config)
  // Create IO based on endpoint's ports
  val io = IO(new Bundle {
    val ports = DataMirror.modulePorts(endpoint).map { case (name, data) =>
    name -> WireInit(data.cloneType, DontCare)
    }.toMap
  })
}

object DiffJsonTop{
  private val instances = ListBuffer.empty[DifftestBundle]
  val instancecount: Int = 0

  def creat_top(config: GatewayConfig): GatewayEndpoint = {
    val json = DataCollector.readFromFile("difftest/diff_class_info.json")
    instantiateModules(json)
    println("creat endpoint")
    val endpoint = Module(new GatewayEndpoint(instances.toSeq, config))
    println("connet endpoint step")
    val step = endpoint.step
    endpoint
  }

  def instantiateModules(jsonData: mutable.Map[String, mutable.Map[String, Any]]): Unit = {
    ModuleRegistry.initialize()
    jsonData.foreach { case (className, params) =>
      println(s"Instantiating module: $className with params: $params")
      val delay = params.get("delay").map(_.asInstanceOf[Double].toInt).getOrElse(0)
      val exceptionCount = params.get("exception_count").map(_.asInstanceOf[Double].toInt).getOrElse(0)
      val numRegs = params.get("numRegs").map(_.asInstanceOf[Double].toInt).getOrElse(0)
      val constructor = ModuleRegistry.getModule(className).getOrElse(throw new Exception(s"Unknown class name: $className"))
      (1 to exceptionCount).foreach { _ =>
        val moduleInstance = constructor(Some(numRegs), delay, true)
        // Automatically set all IO to 1
        InitializePorts(moduleInstance.asInstanceOf[DifftestBundle])
        instances += moduleInstance.asInstanceOf[DifftestBundle]
        println(s"InitializePorts module with instance: $moduleInstance")
      }
    }
  }

  def InitializePorts(bundle: difftest.DifftestBundle): Unit = {
    bundle.elements.foreach {
      case (_, port: chisel3.Bool) => port := DontCare
      case (_, port: UInt) => port := DontCare
      case (_, port: SInt) => port := DontCare
      case (_, port: Vec[_]) => port.foreach(_ := DontCare)
      case _ =>
    }
  }
}

// DifftestModule(new DiffInstrCommit(param1), delay = param2, dontCare = param3)),
// val result = DataCollector.query("DiffArchFpRegState", "delay", "exception_count")
// println(result) // 预期输出：Map("delay" -> Some(2), "exception_count" -> Some(2))

// // 提取值到变量
// val delay: Option[Int] = result.get("delay").flatMap(_.asInstanceOf[Option[Int]])
// val exceptionCount: Option[Int] = result.get("exception_count").flatMap(_.asInstanceOf[Option[Int]])
object ModuleRegistry {

  type ModuleConstructor = (Option[Int], Int, Boolean) => Any

  private val moduleMap: mutable.Map[String, ModuleConstructor] = mutable.Map()

  // Register module
  def register(className: String, constructor: ModuleConstructor): Unit = {
    moduleMap += (className -> constructor)
  }

  // Getting modules
  def getModule(className: String): Option[ModuleConstructor] = {
    moduleMap.get(className)
  }

  // Initialize the module registry
  def initialize(): Unit = {
    register("DiffInstrCommit", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffInstrCommit(param1.getOrElse(0)), delay = param2, dontCare = param3)
    )
    register("DiffArchEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffArchEvent, delay = param2, dontCare = param3)
    )
    register("DiffL1TLBEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffL1TLBEvent)
    )
    register("DiffL2TLBEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffL2TLBEvent)
    )
    register("DiffLrScEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffLrScEvent)
    )
    register("DiffDebugMode", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffDebugMode)
    )
    register("DiffCSRState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffCSRState)
    )
    register("DiffHCSRState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffHCSRState)
    )
    register("DiffVecCSRState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffVecCSRState)
    )
    register("DiffAtomicEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffAtomicEvent)
    )
    register("DiffTrapEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffIntWriteback, dontCare = param3)
    )
    register("DiffRefillEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffRefillEvent, dontCare = param3)
    )
    register("DiffStoreEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffStoreEvent, delay = param2)
    )
    register("DiffLoadEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffLoadEvent, delay = param2)
    )
    register("DiffArchIntRegState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffArchIntRegState, delay = param2)
    )
    register("DiffArchFpRegState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffArchFpRegState, delay = param2)
    )
    register("DiffArchVecRegState", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffArchVecRegState, delay = param2)
    )
    register("DiffSbufferEvent", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffSbufferEvent, delay = param2)
    )
    register("DiffIntWriteback", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffIntWriteback(param1.getOrElse(0)))
    )
    register("DiffFpWriteback", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffFpWriteback(param1.getOrElse(0)))
    )
    register("DiffVecWriteback", (param1: Option[Int], param2: Int, param3: Boolean) =>
      DifftestModule(new DiffVecWriteback(param1.getOrElse(0)))
    )
  }
}
