import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { Scatter } from 'react-chartjs-2';
import "./App.css"

import {
  
  Chart as ChartJS,
  LinearScale,
  
  LineElement,
  CategoryScale,
  PointElement,
} from "chart.js";
import Card from './components/cards';

ChartJS.register(
  LinearScale,
  PointElement,
  LineElement,
  CategoryScale,
);

const YourComponent = () => {
  // Define variables for URLs and polling timings
  const liveStateUrl = 'http://10.243.22.169:8080/latest_state';
  const trendDataUrl = 'http://10.243.22.169:8080/all_data';
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
        console.error('Error fetching data:', error);
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
    console.log(trendData);
  }, [trendData, liveState]);

  // Prepare data for Line chart
  const chartData = {
    datasets: [{
      label: 'Trend Data',
      data: trendData.map(entry => ({
        x: entry.timestamp,
        y: entry.temperature
      })),
      backgroundColor: 'rgba(255, 99, 132, 0.5)', // Example color
      borderColor: 'rgba(255, 99, 132, 1)', // Example color
      borderWidth: 1
    }]
  };

  // Chart options
  const chartOptions = {
    scales: {
      xAxes: [{
        type: 'linear',
        scaleLabel: {
          display: true,
          labelString: 'Timestamp'
        }
      }],
      yAxes: [{
        scaleLabel: {
          display: true,
          labelString: 'State'
        }
      }]
    }
  };

  return (
    <div className='App'>
      <Card/>
      <div className="card">
        <h2>Live State</h2>
        <p>{JSON.stringify(liveState)}</p>
      </div>
      <div className="trend-graph" style={{height:'30rem'}}>
        <Scatter data={chartData} options={chartOptions} />
      </div>
    </div>
  );
};

export default YourComponent;
