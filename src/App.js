import React, { useState, useEffect } from "react";
import axios from "axios";
import { Scatter } from "react-chartjs-2";
import "./App.scss";

import {
  Chart as ChartJS,
  LinearScale,
  LineElement,
  CategoryScale,
  PointElement,
} from "chart.js";
import Card from "./components/cards";

ChartJS.register(LinearScale, PointElement, LineElement, CategoryScale);

const YourComponent = () => {
  // Define variables for URLs and polling timings
  const liveStateUrl = "http://49.205.216.134:8081/latest_state";
  const trendDataUrl = "http://49.205.216.134:8081/all_data";
  const pollingInterval = 1000; // 1 second

  const [liveState, setLiveState] = useState({});
  const [trendData, setTrendData] = useState([]);

  useEffect(() => {
    const fetchData = async () => {
      try {
        // Fetch live state data
        const liveStateResponse = await axios.get(liveStateUrl);
        setLiveState(liveStateResponse.data);

        // Fetch trend data
        const trendDataResponse = await axios.get(trendDataUrl);
        setTrendData(trendDataResponse.data);
      } catch (error) {
        console.error("Error fetching data:", error);
      }
    };

    // Fetch data initially
    fetchData();

    // Poll data every specified interval
    const interval = setInterval(fetchData, pollingInterval);

    // Clean up interval
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    console.log('FILTERED DATA',trendData.filter(entry => entry.temperature > 5 && entry.temperature < 45))
  }, [trendData, liveState]);

  // Prepare data for Line chart
  const chartData = {
    datasets: [
      {
        label: "Trend Data",
        data: trendData
        .filter(entry => entry.temperature > 5 && entry.temperature < 45)
        .map(entry => ({
          x: entry.timestamp,
          y: entry.temperature
        })),
        backgroundColor: "rgba(255, 99, 132, 0.5)", // Example color
        borderColor: "rgba(255, 99, 132, 1)", // Example color
        borderWidth: 1,
      },
    ],
  };

  // Chart options
  // Chart options
  const chartOptions = {
    scales: {
      x: {
        type: "linear",
        title: {
          color: "white",
          display: true,
          text: "Time",
          font: {
            size: 20,
            family: "Montserrat",
          },
        },
        ticks: {
          color: "white",
          callback: convertTimestampTo12HourFormat,
        },
      },
      y: {
        ticks: {
          stepSize: 0.1, // Set step size of y-axis
          color: "white",
        },
        title: {
          color: "white",
          display: true,
          text: "Temperature (*C)",
          font: {
            size: 20,
            family: "Montserrat",
          },
        },
      },
    },
    legend: {
      display: true,
    },
  };
  

  function convertTimestampTo12HourFormat(timestamp) {
    // Create a new Date object with the timestamp in milliseconds
    const date = new Date(timestamp * 1000);

    // Get hours, minutes, and seconds from the Date object
    let hours = date.getHours();
    const minutes = date.getMinutes().toString().padStart(2, '0');
    const seconds = date.getSeconds().toString().padStart(2, '0');
    
    // Determine AM or PM
    const amPM = hours >= 12 ? 'PM' : 'AM';

    // Convert hours to 12-hour format
    hours = hours % 12 || 12;

    // Return the formatted time string
    return `${hours}:${minutes}:${seconds} ${amPM}`;
}

  return (
    <div className="App">
      <div className="App__sidebar">
        <Card title="Temperature" units="*C" value={liveState?.temperature} />
        <Card title="Humidity" units="%" value={liveState?.humidity} />
        <Card
          title="Time"
          units=""
          value={convertTimestampTo12HourFormat(liveState?.timestamp)}
          fontSize={2}
        />
        <Card title="Button State" units="Boolean" value={liveState?.state} />
      </div>

      <div className="App__trend-graph">
        <Scatter data={chartData} options={chartOptions} />
      </div>
    </div>
  );
};

export default YourComponent;
