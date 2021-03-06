import Vue from 'vue'
import Vuex from 'vuex'
import { createLogger } from 'vuex'

import APIService from './services/api.service'
import ConfigService from './services/config.service'
import SystemService from './services/system.service'

const apiService = new APIService();
const configService = new ConfigService(apiService);
const systemService = new SystemService(apiService);

Vue.use(Vuex);

export default new Vuex.Store({
  state: {
    config: null,
    system: null,
  },
  plugins: [
    // XXX: only during development
    createLogger(),
  ],
  actions: {
    /* config */
    async loadConfig({ commit }) {
      const config = await configService.get();

      commit('loadConfig', config);
    },
    async uploadConfig({ dispatch }, file) {
      await configService.upload(file);
      await dispatch('loadConfig');
    },

    /* system */
    async loadSystem({ commit }) {
      const data = await systemService.get();

      commit('updateSystem', data);
    },
    async loadSystemTasks({ commit }) {
      const data = await systemService.getTasks();

      commit('updateSystemTasks', data);
    },
    async restartSystem({ dispatch }) {
      await systemService.restart();

      await dispatch('loadSystem');
    }
  },
  mutations: {
    loadConfig (state, config) {
      state.config = config;
    },
    updateSystem(state, system) {
      state.system = system;
    },
    updateSystemTasks(state, tasks) {
      let prevTasks = new Map();

      if (state.system && state.system.tasks) {
        for (const task of state.system.tasks) {
          prevTasks.set(task.number, task);
        }
      }

      for (const task of tasks) {
        const prevTask = prevTasks.get(task.number);

        if (prevTask) {
          task.prev_runtime = prevTask.runtime;
          task.prev_total_runtime = prevTask.total_runtime;
        }
      }

      state.system.tasks = tasks;
    },
  },
});
